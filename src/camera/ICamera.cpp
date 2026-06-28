#include "ICamera.hpp"
#include <UtilityFunctions.hpp>
#include <parameter/ParameterUtils.hpp>
#include <utility/MoveOnlyFunction.hpp>

ICamera::ICamera(
    const std::string& serialNmb,
    const std::string& interfaceId,
    CameraInterface interfaceType)
    : serialNumber_(serialNmb),
    interfaceId_(interfaceId),
    interfaceType_(interfaceType)
{
}

ICamera::~ICamera()
{
    // TODO: log?
    /*TaskResult shutdownRes = */doShutdown();
}

//////////////////////////////////////////////

TaskResult ICamera::shutdown()
{
    //TODO: class-wide flag for camera dead -> cannot call "setters"

    if (isShuttingDown_.exchange(true))
    {
        return { false, { "Already trying to shutting down", Log::Level::LVL_INFO } };
    }

    return doShutdown();
}

TaskEnqueueResult ICamera::check()
{
    enqueueMessageTask(
        MSG_CHECK_CAMERA,
        SourceId::none(),
        { { CAM_SERIAL, serialNumber() } },
        &ICamera::doCheck,
        this
    );

    return TaskEnqueueResult::Status::NO_ERRORS;
}

TaskEnqueueResult ICamera::connect()
{
    if (!isDetected())
    {
        return { TaskEnqueueResult::Status::REJECTED, { "Camera not detected", Log::Level::LVL_ERROR } };
    }

    if (isConnected())
    {
        return { TaskEnqueueResult::Status::ALREADY_DONE, { "Camera already connected",  Log::Level::LVL_INFO } };
    }

    if (isConnecting_.exchange(true))
    {
        return { TaskEnqueueResult::Status::ALREADY_ACTIVE, { "Already trying to connect", Log::Level::LVL_INFO } };
    }

    enqueueMessageTask(
        MSG_CONNECT_CAMERA,
        SourceId::none(),
        { { CAM_SERIAL, serialNumber() } },
        &ICamera::doConnect,
        this
    );

    return TaskEnqueueResult::Status::NO_ERRORS;
}

TaskEnqueueResult ICamera::disconnect()
{
    if (isGrabbing())
    {
        return { TaskEnqueueResult::Status::REJECTED, { "Cannot disconnect while grabbing", Log::Level::LVL_ERROR } };
    }

    if (!isConnected())
    {
        return { TaskEnqueueResult::Status::ALREADY_DONE, { "Camera is not connected", Log::Level::LVL_INFO } };
    }

    if (isDisconnecting_.exchange(true))
    {
        return { TaskEnqueueResult::Status::ALREADY_ACTIVE, { "Already trying to disconnect", Log::Level::LVL_INFO } };
    }

    enqueueMessageTask(
        MSG_DISCONNECT_CAMERA,
        SourceId::none(),
        { { CAM_SERIAL, serialNumber() } },
        &ICamera::doDisconnect,
        this
    );

    return TaskEnqueueResult::Status::NO_ERRORS;
}

TaskEnqueueResult ICamera::startGrab(int timeout)
{
    if (!isConnected())
    {
        return { TaskEnqueueResult::Status::REJECTED, { "Camera is not connected", Log::Level::LVL_ERROR } };
    }

    if (isGrabbing())
    {
        return { TaskEnqueueResult::Status::ALREADY_DONE, { "Already grabbing", Log::Level::LVL_INFO } };
    }

    if (isStartingGrab_.exchange(true))
    {
        return { TaskEnqueueResult::Status::ALREADY_ACTIVE, { "Already trying to start grab", Log::Level::LVL_INFO } };
    }

    MessageTaskScoped msg(MSG_START_GRABBING, msgDispatcher_, SourceId::none(), { { CAM_SERIAL, serialNumber() } });

    worker_.enqueueTask([this, timeout, msg = std::move(msg)]() mutable {

        msg.start();

        TaskResult setAcqModeRes = setAcquisitionMode(AcquisitionMode::CONTINUOUS);
        std::vector<Log> exitLogs = setAcqModeRes.takeLogs();

        if (msg.abortIf(!setAcqModeRes.isSuccess(), std::move(exitLogs)))
        {
            isStartingGrab_.store(false);
            return false;
        }


        // Test if camera can grab images
        size_t retry_count = 3;
        TaskResultP<std::shared_ptr<const CvImage>> result;

        TaskResult startRes = startFrameCapture();
        UtilityFunctions::moveInto(startRes.takeLogs(), exitLogs);


        if (msg.abortIf(!startRes.isSuccess(), std::move(exitLogs)))
        {
            isStartingGrab_.store(false);
            return false;
        }

        do
        {
            result = acquireFrame(timeout);

        } while (!result.isSuccess() && --retry_count > 0);

        UtilityFunctions::moveInto(result.takeLogs(), exitLogs);

        if (!result.isSuccess())
        {
            isStartingGrab_.store(false);

            TaskResult stopResult = stopFrameCapture();
            UtilityFunctions::moveInto(stopResult.takeLogs(), exitLogs);

            msg.abort(std::move(exitLogs));
            return false;
        }

        // So the buffer can have at least one image ready
        addImageToBuffer(result.getPayload());

        try
        {
            grabState();
        }
        catch (const std::logic_error& ex)
        {
            exitLogs.push_back(
                { std::string("State transition error: ") + ex.what(), Log::Level::LVL_ERROR }
            );
        }

        msg.complete(std::move(exitLogs));

        isStartingGrab_.store(false);

        scheduleNextGrab(timeout);

        return true;

        });

    return TaskEnqueueResult::Status::NO_ERRORS;
}

TaskEnqueueResult ICamera::stopGrab()
{
    if (!isConnected())
    {
        return { TaskEnqueueResult::Status::REJECTED, { "Camera is not connected", Log::Level::LVL_ERROR } };
    }

    if (!isGrabbing())
    {
        return { TaskEnqueueResult::Status::ALREADY_DONE, { "Camera is not grabbing", Log::Level::LVL_INFO } };
    }

    if (isStoppingGrab_.exchange(true))
    {
        return { TaskEnqueueResult::Status::ALREADY_ACTIVE, { "Already trying to stop grab", Log::Level::LVL_INFO } };
    }

    requestStopGrabbing_.store(true);

    return TaskEnqueueResult::Status::NO_ERRORS;
}

TaskEnqueueResult ICamera::takeSnapshot(
    MoveOnlyFunction<void(const TaskResultP<std::shared_ptr<const CvImage>>&)> func,
    int timeout)
{
    if (!isConnected())
    {
        return { TaskEnqueueResult::Status::REJECTED, { "Camera is not connected", Log::Level::LVL_ERROR } };
    }

    if (isGrabbing())
    {
        std::lock_guard lock(snapshotRequestsMutex_);

        snapshotRequests_.push_back({
            std::move(func),
            std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout)
            });

        return TaskEnqueueResult::Status::NO_ERRORS;
    }

    //worker_.enqueueTask(
    //    &ICamera::doTakeSnapshot,
    //    this,
    //    std::move(func),
    //    timeout
    //);

    worker_.enqueueTask(
        [this,
        func = std::move(func),
        timeout]() mutable
        {
            doTakeSnapshot(std::move(func), timeout);
        });

    return TaskEnqueueResult::Status::NO_ERRORS;
}

std::shared_ptr<const CvImage> ICamera::getLatestFrame()
{
    std::unique_lock<std::shared_mutex> lock(bufferMutex_); // Acquire exclusive (write) lock

    return imageBuffer_.empty() ? nullptr : imageBuffer_.back();
}

TaskResult ICamera::setParameter(
    const std::string& paramId,
    const std::string& categoryId,
    const std::string& value)
{
    if (!isConnected())
    {
        return { false, { "Camera is not connected", Log::Level::LVL_ERROR } };
    }


    TaskResult sdkRes = setStringParameter(paramId, value);
    std::vector<Log> exitLogs = sdkRes.takeLogs();
	bool success = sdkRes.isSuccess();
    
	if (success)
    {
        TaskResult result = ParameterOwner::setParameter(paramId, categoryId, value);
        UtilityFunctions::moveInto(result.takeLogs(), exitLogs);
    }

    return { success, std::move(exitLogs) };
}

TaskResult ICamera::setParameter(
    const std::string& paramId,
    const std::string& categoryId,
    int value)
{
    if (!isConnected())
    {
        return { false, { "Camera is not connected", Log::Level::LVL_ERROR } };
    }


    TaskResult sdkRes = setIntParameter(paramId, value);
    std::vector<Log> exitLogs = sdkRes.takeLogs();
	bool success = sdkRes.isSuccess();
    
	if (success)
    {
        TaskResult result = ParameterOwner::setParameter(paramId, categoryId, value);
        UtilityFunctions::moveInto(result.takeLogs(), exitLogs);
    }

    return { success, std::move(exitLogs) };
}

TaskResult ICamera::setParameter(
    const std::string& paramId,
    const std::string& categoryId,
    double value)
{
    if (!isConnected())
    {
        return { false, { "Camera is not connected", Log::Level::LVL_ERROR } };
    }


    TaskResult sdkRes = setDoubleParameter(paramId, value);
    std::vector<Log> exitLogs = sdkRes.takeLogs();
	bool success = sdkRes.isSuccess();
    
	if (success)
    {
        TaskResult result = ParameterOwner::setParameter(paramId, categoryId, value);
        UtilityFunctions::moveInto(result.takeLogs(), exitLogs);
    }

    return { success, std::move(exitLogs) };
}

TaskResult ICamera::setParameter(
    const std::string& paramId,
    const std::string& categoryId,
    bool value)
{
    if (!isConnected())
    {
        return { false, { "Camera is not connected", Log::Level::LVL_ERROR } };
    }


    TaskResult sdkRes = setBoolParameter(paramId, value);
    std::vector<Log> exitLogs = sdkRes.takeLogs();
	bool success = sdkRes.isSuccess();
    
	if (success)
    {
        TaskResult result = ParameterOwner::setParameter(paramId, categoryId, value);
        UtilityFunctions::moveInto(result.takeLogs(), exitLogs);
    }

    return { success, std::move(exitLogs) };
}

TaskResult ICamera::executeCommand(
    const std::string& paramId,
    const std::string& categoryId)
{
    if (!isConnected())
    {
        return { false, { "Camera is not connected", Log::Level::LVL_ERROR } };
    }

    return executeCommand(paramId);
}

TaskResult ICamera::resetParameter(
    const std::string& paramId,
    const std::string& categoryId)
{
    if (!isConnected())
    {
        return { false, { "Camera is not connected", Log::Level::LVL_ERROR } };
    }

	std::shared_ptr<ParameterInfo> defaultParam = getDefaultParameter(paramId, categoryId);
    if (!defaultParam)
    {
        return { false, { "Parameter " + paramId + " in category " + categoryId + " does not exists in cache", Log::Level::LVL_ERROR } };
    }

    TaskResult sdkRes;
    switch (defaultParam->dataType())
    {
        case DataType::INTEGER:
			return setParameter(paramId, categoryId, defaultParam->getValue<int>());

		case DataType::DOUBLE:
			return setParameter(paramId, categoryId, defaultParam->getValue<double>());

		case DataType::ENUMERATION:
			return setParameter(paramId, categoryId, defaultParam->getValue<int>());

		case DataType::BOOLEAN:
			return setParameter(paramId, categoryId, defaultParam->getValue<bool>());

		case DataType::STRING:
			return setParameter(paramId, categoryId, defaultParam->getValue<std::string>());

        default:
            break;
    }

    return { false, { "Parameter " + paramId + " in category " + categoryId + " has not resetable type", Log::Level::LVL_ERROR } };
}

TaskResult ICamera::resetDirtyParameters()
{
    return TaskResult();
}

std::shared_ptr<ICameraInfo> ICamera::getCameraView() const
{
    return std::make_shared<ICameraInfoImpl>(std::static_pointer_cast<const ICamera>(shared_from_this()));
}

bool ICamera::isDetected() const
{
    std::shared_lock lock(stateMutex_);
    return stateMachine_.state().presence() == CameraState::Presence::DETECTED;
}

bool ICamera::isConnected() const
{
    std::shared_lock lock(stateMutex_);
    return stateMachine_.state().connection() == CameraState::Connection::CONNECTED;
}

bool ICamera::isGrabbing() const
{
    std::shared_lock lock(stateMutex_);
    return stateMachine_.state().activity() == CameraState::Activity::GRABBING;
}

bool ICamera::disconnectionRequested() const
{
    return isDisconnecting_.load();
}

void ICamera::setParametersPersistence(bool arePersistent)
{
    persistentParams_.store(arePersistent);
};

bool ICamera::areParametersPersistent() const
{
    return persistentParams_.load();
};

CameraState ICamera::state() const
{
    std::shared_lock lock(stateMutex_);
    return stateMachine_.state();
}

const std::string& ICamera::interfaceId() const
{
    return interfaceId_;
};

const std::string& ICamera::serialNumber() const
{
    return serialNumber_;
};

CameraInterface ICamera::interfaceType() const
{
    return interfaceType_;
}

///////////////////////////////////////////////////////////////////////////////////////

TaskResult ICamera::doShutdown()
{
    MessageTaskScoped msg(MSG_SHUTDOWN_CAMERA, msgDispatcher_, SourceId::none(), { { CAM_SERIAL, serialNumber() } });

    msg.start();

    std::vector<Log> exitLogs;

    if (isGrabbing())
    {
        TaskResult res = stopGrabSync();
        exitLogs = res.takeLogs();

        try
        {
            idleState();
        }
        catch (const std::logic_error& ex)
        {
            std::string err = ex.what();
            exitLogs.push_back(err);
        }
    }

    if (isConnected())
    {
        TaskResult res = disconnectSync();
        UtilityFunctions::moveInto(res.takeLogs(), exitLogs);

        try
        {
            disconnectedState();
        }
        catch (const std::logic_error& ex)
        {
            std::string err = ex.what();
            exitLogs.push_back(err);
        }
    }

    AsyncWorker::shutdown();

    msg.complete(exitLogs);

    return { true, std::move(exitLogs) };
}

TaskResultP<CameraState> ICamera::doCheck()
{
    static constexpr size_t MAX_RETRIES = 5;
    static constexpr auto RETRY_DELAY = std::chrono::milliseconds(100);

    TaskResult result;
    for (size_t retry = 0; retry < MAX_RETRIES; ++retry)
    {
        if (isConnected())
        {
            result = checkConnectionHealth();
        }
        else
        {
            result = checkCamera();
        }

        if (result.isSuccess())
            break;

        std::this_thread::sleep_for(RETRY_DELAY);
    }

    std::vector<Log> exitLogs = result.takeLogs();

    if (result.isSuccess())
    {
        try
        {
            detectedState();
        }
        catch (const std::logic_error& ex)
        {
            exitLogs.push_back(
                { std::string("State transition error: ") + ex.what(), Log::Level::LVL_ERROR }
            );
        }   
    }
    else  // "Ci ho provato, ma non riesco a trovarla. Ferma tutto e diciamo che la camera non si trova più"
    {
        if (isGrabbing())
        {
            if (isStoppingGrab_.load())
            {
                static constexpr auto RETRY_DELAY = std::chrono::milliseconds(500);
                while (isGrabbing())
                {
                    if (!isStoppingGrab_.load())
                    {
                        if (isGrabbing())
                        {
                            exitLogs.push_back(
                                { "Stopping grab failed by another thread!", Log::Level::LVL_WARNING }
                            );
                        }
                        break;
                    }
                    std::this_thread::sleep_for(RETRY_DELAY);
                }
            }
            else
            {
                TaskResult stopGrabRes = stopGrabSync();

                UtilityFunctions::moveInto(stopGrabRes.takeLogs(), exitLogs);
            }
        }

        if (isConnected())
        {
            if (isDisconnecting_.load())
            {
                static constexpr auto RETRY_DELAY = std::chrono::milliseconds(500);
                while (isConnected())
                {
                    if (!isDisconnecting_.load())
                    {
                        if (isConnected())
                        {
                            exitLogs.push_back(
                                { "Disconnecting failed by another thread!", Log::Level::LVL_WARNING }
                            );
                        }
                        break;
                    }
                    std::this_thread::sleep_for(RETRY_DELAY);
                }
            }
            else
            {
                TaskResult disconnectRes = disconnectSync();

                UtilityFunctions::moveInto(disconnectRes.takeLogs(), exitLogs);
            }
        }

        try
        {
            notDetectedState();
        }
        catch (const std::logic_error& ex)
        {
            exitLogs.push_back(
                { std::string("State transition error: ") + ex.what(), Log::Level::LVL_ERROR }
            );
        }
    }


    return { state(), std::move(exitLogs) };
}

TaskResult ICamera::doConnect()
{
    // --- Step 1: Connecting ---
    TaskResult connectionResult = connectCamera();
    std::vector<Log> exitLogs = connectionResult.takeLogs();

    if (connectionResult.isSuccess())
    {
        try
        {
            connectedState();
        }
        catch (const std::logic_error& ex)
        {
            exitLogs.push_back(
                { std::string("State transition error: ") + ex.what(), Log::Level::LVL_ERROR }
            );
        }

        // --- Step 2: Loading Params (only if connected successfully) ---
        clearAllParameters();

        std::vector<std::shared_ptr<Parameter>> features;
        TaskResult loadingParamsResult = loadCameraParams(features);
        UtilityFunctions::moveInto(loadingParamsResult.takeLogs(), exitLogs);

        if (loadingParamsResult.isSuccess())
        {
            initParameters(features);
        }
    }
    else
    {
        // Connection failed, skip param loading
        exitLogs.push_back(
            { "Failed to connect to device", Log::Level::LVL_WARNING }
        );
    }

    isConnecting_.store(false);

    return { connectionResult.isSuccess(), std::move(exitLogs) };
}

TaskResult ICamera::doDisconnect()
{
    std::vector<Log> exitLogs;

    if (!areParametersPersistent())
    {
        std::vector<ParamKey> local_editedParams = getDirtyParams();

        for (const auto& [paramId, categoryId] : local_editedParams)
        {
            TaskResult r = resetParameter(paramId, categoryId);

            UtilityFunctions::moveInto(r.takeLogs(), exitLogs);
        }
    }

    TaskResult result = disconnectCamera();
    UtilityFunctions::moveInto(result.takeLogs(), exitLogs);

    if (result.isSuccess())
    {
        try
        {
            disconnectedState();
        }
        catch (const std::logic_error& ex)
        {
            exitLogs.push_back(
                { std::string("State transition error: ") + ex.what(), Log::Level::LVL_ERROR }
            );
        }
    }

    isDisconnecting_.store(false);

    return { result.isSuccess(), exitLogs };
}

TaskResult ICamera::doRequestStopGrabLoop()
{
    constexpr int maxWaitTimeMs = 20000; // 20 seconds timeout
    constexpr int checkIntervalMs = 100; // Check every 100ms
    int elapsedTimeMs = 0;

    requestStopGrabbing_.store(true);

    while (isGrabbing() && elapsedTimeMs < maxWaitTimeMs)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(checkIntervalMs));
        elapsedTimeMs += checkIntervalMs;
    }

    requestStopGrabbing_.store(false);
    isStoppingGrab_.store(false);

    if (!isGrabbing())
    {
        // Successfully stopped
        return true;
    }
    else
    {
        // Process didn't stop within the timeout period
        return { false, { "Failed to stop Image grabbing process (timeout " + std::to_string(maxWaitTimeMs / 1000) + "s).", Log::Level::LVL_ERROR } };
    }
}

void ICamera::doTakeSnapshot(
    MoveOnlyFunction<void(const TaskResultP<std::shared_ptr<const CvImage>>&)> func,
    int timeout)
{
    //
    TaskResult setAcqModeRes = setAcquisitionMode(AcquisitionMode::SINGLE_FRAME);
    auto exitLogs = setAcqModeRes.takeLogs();

    if (!setAcqModeRes.isSuccess())
    {
        func({ std::nullopt, std::move(exitLogs) });

        return;
    }

    //
    TaskResult startAcqRes = startFrameCapture();
    UtilityFunctions::moveInto(startAcqRes.takeLogs(), exitLogs);

    if (!startAcqRes.isSuccess())
    {
        func({ std::nullopt, std::move(exitLogs) });

        return;
    }

    //
    TaskResultP<std::shared_ptr<const CvImage>> acqResult = acquireFrame(timeout);
    UtilityFunctions::moveInto(acqResult.takeLogs(), exitLogs);

    //
    TaskResult stopAcqRes = stopFrameCapture();
    UtilityFunctions::moveInto(stopAcqRes.takeLogs(), exitLogs);

    //
	const bool success = acqResult.isSuccess();
    if (success)
    {
        const std::shared_ptr<const CvImage>& snap = acqResult.getPayload();

        func({ snap, std::move(exitLogs) });

        addImageToBuffer(snap);

        return;
    }
}

void ICamera::scheduleNextGrab(
    int timeout,
    int consecutiveGrabFails)
{
    if (!requestStopGrabbing_.load())
    {
        worker_.enqueueTask(&ICamera::doNextGrab, this, timeout, consecutiveGrabFails);
    }
    else
    {
        finalizeGrabLoop();
    }
}

void ICamera::doNextGrab(
    int timeout,
    int consecutiveGrabFails)
{
    TaskResultP<std::shared_ptr<const CvImage>> acqResult = acquireFrame(timeout);


    // Live frames

    if (!acqResult.isSuccess())
    {
        ++consecutiveGrabFails;

        // TODO: 
        if (consecutiveGrabFails >= 3)
        {
            consecutiveGrabFails = 0;

            std::vector<Log> exitLogs = acqResult.takeLogs();

            exitLogs.push_back({ "Stopping camera grabbing because 3 consecutive fails", Log::Level::LVL_ERROR });

            publish(Message(
                MSG_ACQUIRED_FRAME_FAIL,
                SourceId::none(),
                { {CAM_SERIAL, serialNumber()} },
                std::move(exitLogs)
            ));

            stopGrab();
        }
        else
        {
            // Publish failed acquisition
            publish(Message(
                MSG_ACQUIRED_FRAME_FAIL,
                SourceId::none(),
                { { CAM_SERIAL, serialNumber() } },
                acqResult.takeLogs()
            ));
        }
    }
    else
    {
        addImageToBuffer(acqResult.getPayload());
        consecutiveGrabFails = 0;

        // Publish successful acquisition
        publish(Message(
            MSG_ACQUIRED_FRAME,
            SourceId::none(),
            { { CAM_SERIAL, serialNumber() } },
            acqResult.takeLogs()
        ));
    }


    // Snapshots

    std::deque<SnapshotRequest> pending;

    {
        std::lock_guard lock(snapshotRequestsMutex_);
        pending.swap(snapshotRequests_);
    }

    auto now = std::chrono::steady_clock::now();

    for (auto& req : pending)
    {
        if (req.deadline < now)
        {
            req.func( { std::nullopt, { "Snapshot request timeout", Log::Level::LVL_ERROR } } );
        }
        else
        {
            req.func(acqResult);
        }
    }

    scheduleNextGrab(timeout, consecutiveGrabFails);
}

void ICamera::finalizeGrabLoop()
{
    // TODO: now is always success. Should be? I think at least it cannot be a total fail, as if we are here, ICamera grab loop is actually killed,
    // but if 'stopFrameCapture' return false, camera hardware can still be in a physical acquiring state.

    MessageTaskScoped msg(MSG_STOP_GRABBING, msgDispatcher_, SourceId::none(), { { CAM_SERIAL, serialNumber() } });
    msg.start();

    TaskResult stopGrabResult = stopFrameCapture();

    if (isStoppingGrab_.load())
        isStoppingGrab_.store(false);

    if (requestStopGrabbing_.load())
        requestStopGrabbing_.store(false);

    std::vector<Log> exitLogs = stopGrabResult.takeLogs();

    try
    {
        idleState();
    }
    catch (const std::logic_error& ex)
    {
        exitLogs.push_back(
            { std::string("State transition error : ") + ex.what(), Log::Level::LVL_ERROR }
        );
    }

    if (!stopGrabResult.isSuccess())
    {
        exitLogs.push_back(
            { "Error while stopping underlying camera acquisition", Log::Level::LVL_WARNING }
        );

        UtilityFunctions::moveInto(stopGrabResult.takeLogs(), exitLogs);
    }

    msg.complete(std::move(exitLogs));
}

TaskResult ICamera::stopGrabSync()
{
    if (!isConnected())
    {
        return { false, { "Camera is not connected", Log::Level::LVL_ERROR } };
    }

    if (!isGrabbing())
    {
        return { true, { "Camera is not grabbing", Log::Level::LVL_INFO } };
    }

    if (isStoppingGrab_.exchange(true))
    {
        // TODO: returning true is misleading

        return { true, { "Already trying to stop grab", Log::Level::LVL_INFO } };
    }


    MessageTaskScoped msg(MSG_STOP_GRABBING, msgDispatcher_, SourceId::none(), { {CAM_SERIAL, serialNumber()} });

    msg.start();

    TaskResult res = doRequestStopGrabLoop();

    if (res.isSuccess())
    {
        msg.complete(res.takeLogs());
    }
    else
    {
        msg.abort(res.takeLogs());
    }

    return res;
}

TaskResult ICamera::disconnectSync()
{
    if (isGrabbing())
    {
        return { false, { "Cannot disconnect while grabbing", Log::Level::LVL_ERROR } };
    }

    if (!isConnected())
    {
        return { true, { "Camera is not connected", Log::Level::LVL_INFO } };
    }

    if (isDisconnecting_.exchange(true))
    {
        // TODO: returning true is misleading

        return { true, { "Already trying to disconnect", Log::Level::LVL_INFO } };
    }


    MessageTaskScoped msg(MSG_DISCONNECT_CAMERA, msgDispatcher_, SourceId::none(), { { CAM_SERIAL, serialNumber() } });

    msg.start();

    TaskResult res = doDisconnect();

    if (res.isSuccess())
    {
        msg.complete(res.takeLogs());
    }
    else
    {
        msg.abort(res.takeLogs());
    }

    return res;
}

void ICamera::addImageToBuffer(const std::shared_ptr<const CvImage>& newImage)
{
    std::unique_lock<std::shared_mutex> lock(bufferMutex_); // Acquire exclusive (write) lock

    if (imageBuffer_.size() >= MAX_BUFFER_SIZE)
    {
        imageBuffer_.pop_front();
    }

    imageBuffer_.push_back(newImage);
}

void ICamera::detectedState()
{
    std::unique_lock lock(stateMutex_);
    stateMachine_ = stateMachine_.detected();
}

void ICamera::notDetectedState()
{
    std::unique_lock lock(stateMutex_);
    stateMachine_ = stateMachine_.notDetected();
}

void ICamera::connectedState()
{
    std::unique_lock lock(stateMutex_);
    stateMachine_ = stateMachine_.connected();
}

void ICamera::disconnectedState()
{
    std::unique_lock lock(stateMutex_);
    stateMachine_ = stateMachine_.disconnected();
}

void ICamera::grabState()
{
    std::unique_lock lock(stateMutex_);
    stateMachine_ = stateMachine_.grabbing();
}

void ICamera::idleState()
{
    std::unique_lock lock(stateMutex_);
    stateMachine_ = stateMachine_.idle();
}

std::shared_ptr<const ParameterOwner> ICamera::getSharedParameterOwner() const
{
    return std::static_pointer_cast<const ParameterOwner>(
        std::static_pointer_cast<const ICamera>(shared_from_this())
    );
}


///////////////////////////////////////////////////////////////////////////////////////


ICamera::ICameraInfoImpl::ICameraInfoImpl(const std::shared_ptr<const ICamera>& cam)
    : ParameterOwnerInfoImpl(cam),
    cam_(cam)
{
}

///////////////////////////////////////////////////////////////////////////////////////

bool ICamera::ICameraInfoImpl::isDetected() const
{
    return cam_->isDetected();
}

bool ICamera::ICameraInfoImpl::isConnected() const
{
    return cam_->isConnected();
}

bool ICamera::ICameraInfoImpl::isGrabbing() const
{
    return cam_->isGrabbing();
}

bool ICamera::ICameraInfoImpl::disconnectionRequested() const
{
    return cam_->disconnectionRequested();
}

CameraState ICamera::ICameraInfoImpl::state() const
{
    return cam_->state();
}

bool ICamera::ICameraInfoImpl::areParametersPersistent() const
{
    return cam_->areParametersPersistent();
}

const std::string& ICamera::ICameraInfoImpl::interfaceId() const
{
    return cam_->interfaceId();
}

const std::string& ICamera::ICameraInfoImpl::serialNumber() const
{
    return cam_->serialNumber();
}

CameraInterface ICamera::ICameraInfoImpl::interfaceType() const
{
    return cam_->interfaceType();
}