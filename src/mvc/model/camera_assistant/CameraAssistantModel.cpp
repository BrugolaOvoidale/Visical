#include "CameraAssistantModel.hpp"
#include <UtilityFunctions.hpp>
#include <camera_manager/CameraManager.hpp>
#include <camera_manager/ICameraProxy.hpp>
#include <camera/CameraUtils.hpp>
#include <camera/interfaces/aravis/AravisGenICamCamera.hpp>
#include <camera/interfaces/openCV/OpenCVCamera.hpp>


//////////////////////////////////////////////////////////////

CameraAssistantModel::CameraAssistantModel(const std::shared_ptr<MessageLogger>& logger) : BaseModel(logger)
{
    cameraManager_ = std::make_shared<CameraManager>();

	init();
}

CameraAssistantModel::CameraAssistantModel(
    const std::shared_ptr<CameraManager>& cameraManager,
    const std::shared_ptr<MessageLogger>& logger) : BaseModel(logger)
{
    if (!cameraManager)
    {
		throw std::invalid_argument("cameraManager is nullptr");
    }

	cameraManager_ = cameraManager;

    init();
}

CameraAssistantModel::~CameraAssistantModel()
{
    shutdown();
}

//////////////////////////////////////////////////////////////

void CameraAssistantModel::shutdown()
{
    cameraManager_->unsubscribe(MSG_CAM_MGR_REGISTER_CAM, &CameraAssistantModel::onRegisterCamera, this);
    cameraManager_->unsubscribe(MSG_CAM_MGR_UNREGISTER_CAM, &CameraAssistantModel::onUnregisterCamera, this);

    cameraManager_->unregisterAllCameras();
}

TaskEnqueueResult CameraAssistantModel::autoDiscovery()
{
    enqueueMessageTask(
        MSG_CAM_ASSIST_AUTO_DISCOVERY,
        SourceId::none(),
        {},
        &CameraAssistantModel::autoDiscoveryTask,
        this
    );

    return TaskEnqueueResult::Status::NO_ERRORS;
}

TaskEnqueueResult CameraAssistantModel::discoverInterface(CameraInterface interfaceType)
{
    enqueueMessageTask(
        MSG_CAM_ASSIST_DISCOVER_INTERFACE,
        SourceId::none(),
        {},
        &CameraAssistantModel::discoverInterfaceTask,
        this,
        interfaceType
    );

    return TaskEnqueueResult::Status::NO_ERRORS;
}

TaskEnqueueResult CameraAssistantModel::loadCamerasConfig(const std::vector<CameraDTO>& cameraVec)
{
    if (cameraVec.empty())
    {
        return {
            TaskEnqueueResult::Status::REJECTED,
            { "No cameras provided", Log::Level::LVL_ERROR }
        };
    }

    enqueueMessageTask(
        MSG_CAM_ASSIST_LOAD_CAM_FROM_CONFIG,
        SourceId::none(),
        {},
        &CameraAssistantModel::loadCamerasConfigTask,
        this,
        cameraVec
    );

    return TaskEnqueueResult::Status::NO_ERRORS;
}

TaskEnqueueResult CameraAssistantModel::checkCamera(const std::string& cameraId)
{
    std::shared_ptr<ICameraProxy> camera = cameraManager_->getCameraProxy(cameraId);

    if (!camera)
    {
        return {
            TaskEnqueueResult::Status::REJECTED,
            { "Camera not found", Log::Level::LVL_ERROR }
        };
    }

    return camera->check();
}

TaskEnqueueResult CameraAssistantModel::connectCamera(const std::string& cameraId)
{
    std::shared_ptr<ICameraProxy> camera = cameraManager_->getCameraProxy(cameraId);

    if (!camera)
    {
        return {
            TaskEnqueueResult::Status::REJECTED,
            { "Camera not found", Log::Level::LVL_ERROR }
        };
    }
    
    return camera->connect();
}

TaskEnqueueResult CameraAssistantModel::disconnectCamera(const std::string& cameraId)
{
    std::shared_ptr<ICameraProxy> camera = cameraManager_->getCameraProxy(cameraId);

    if (!camera)
    {
        return {
            TaskEnqueueResult::Status::REJECTED,
            { "Camera not found", Log::Level::LVL_ERROR }
        };
    }

    return camera->disconnect();
}

std::shared_ptr<ICameraInfo> CameraAssistantModel::getCamera(const std::string& cameraId) const
{
    std::shared_ptr<ICameraProxy> camera = cameraManager_->getCameraProxy(cameraId);

    if (!camera)
    {
        return nullptr;
    }

    return camera->getCameraView();
}

std::vector<CameraAssistantModel::CameraDescriptorInfo> CameraAssistantModel::getAllCameras() const
{
    std::shared_lock lock(registeredCamerasMutex_);

    std::vector<CameraAssistantModel::CameraDescriptorInfo> camViews;
    camViews.reserve(registeredCameras_.size());

    for (const auto& descr : registeredCameras_)
    {
        std::shared_ptr<ICameraProxy> camera = cameraManager_->getCameraProxy(descr.first);

        if (!camera)
            continue;

        camViews.emplace_back(camera->getCameraView(), descr.second.behaviour);
    }

    return camViews;
}

TaskResult CameraAssistantModel::setCameraParameter(
    const std::string& cameraId,
    const std::string& paramId,
    const std::string& categoryId,
    const std::string& value)
{
    std::shared_ptr<ICameraProxy> camera = cameraManager_->getCameraProxy(cameraId);

    if (!camera)
    {
        return {
            false,
            { "Camera not found", Log::Level::LVL_ERROR }
        };
    }
    
    return camera->setParameter(paramId, categoryId, value);
}

TaskResult CameraAssistantModel::setCameraParameter(
    const std::string& cameraId,
    const std::string& paramId,
    const std::string& categoryId,
    int value)
{
    std::shared_ptr<ICameraProxy> camera = cameraManager_->getCameraProxy(cameraId);

    if (!camera)
    {
        return {
            false,
            { "Camera not found", Log::Level::LVL_ERROR }
        };
    }
    
    return camera->setParameter(paramId, categoryId, value);
}

TaskResult CameraAssistantModel::setCameraParameter(
    const std::string& cameraId,
    const std::string& paramId,
    const std::string& categoryId,
    double value)
{
    std::shared_ptr<ICameraProxy> camera = cameraManager_->getCameraProxy(cameraId);

    if (!camera)
    {
        return {
            false,
            { "Camera not found", Log::Level::LVL_ERROR }
        };
    }
    
    return camera->setParameter(paramId, categoryId, value);
}

TaskResult CameraAssistantModel::setCameraParameter(
    const std::string& cameraId,
    const std::string& paramId,
    const std::string& categoryId,
    bool value)
{
    std::shared_ptr<ICameraProxy> camera = cameraManager_->getCameraProxy(cameraId);

    if (!camera)
    {
        return {
            false,
            { "Camera not found", Log::Level::LVL_ERROR }
        };
    }
    
    return camera->setParameter(paramId, categoryId, value);
}

TaskResult CameraAssistantModel::executeCameraCommand(
    const std::string& cameraId,
    const std::string& paramId,
    const std::string& categoryId)
{
    std::shared_ptr<ICameraProxy> camera = cameraManager_->getCameraProxy(cameraId);

    if (!camera)
    {
        return {
            false,
            { "Camera not found", Log::Level::LVL_ERROR }
        };
    }
    
    return camera->executeCommand(paramId, categoryId);
}

TaskResult CameraAssistantModel::resetCameraParameter(
    const std::string& cameraId,
    const std::string& paramId,
    const std::string& categoryId)
{
    std::shared_ptr<ICameraProxy> camera = cameraManager_->getCameraProxy(cameraId);

    if (!camera)
    {
        return {
            false,
            { "Camera not found", Log::Level::LVL_ERROR }
        };
    }

    return camera->resetParameter(paramId, categoryId);
}

TaskResult CameraAssistantModel::setCameraParamsPersistence(
    const std::string& cameraId,
    bool arePersistent)
{
    std::shared_ptr<ICameraProxy> camera = cameraManager_->getCameraProxy(cameraId);

    if (!camera)
    {
        return {
            false,
            { "Camera not found", Log::Level::LVL_ERROR }
        };
    }

    camera->setParametersPersistence(arePersistent);

    return true;
}

bool CameraAssistantModel::setCameraBehaviour(
    const std::string& cameraId,
    CameraAssistantModel::CameraBehaviour beahviour)
{
    std::shared_lock lock(registeredCamerasMutex_);

    auto it = registeredCameras_.find(cameraId);
    if (it == registeredCameras_.end())
        return false;

    it->second.behaviour = beahviour;

    return true;
}

void CameraAssistantModel::setDefaultCameraBehaviour(CameraBehaviour beahviour)
{
    return defaultCamBehaviour_.store(beahviour);
}

CameraAssistantModel::CameraBehaviour CameraAssistantModel::getDefaultCameraBehaviour() const
{
    return defaultCamBehaviour_.load();
}

//////////////////////////////////////////////////////////////

void CameraAssistantModel::init()
{
    cameraManager_->subscribe(MSG_CAM_MGR_REGISTER_CAM, &CameraAssistantModel::onRegisterCamera, this);
    cameraManager_->subscribe(MSG_CAM_MGR_UNREGISTER_CAM, &CameraAssistantModel::onUnregisterCamera, this);
}

TaskResult CameraAssistantModel::autoDiscoveryTask()
{
#ifdef WITH_GENICAM
    // Aravis
    {
        std::vector<CameraUtils::Descriptor> arvDevices = CameraUtils::enumerateAravis();

        for (const auto& descr : arvDevices)
        {
            addPendingRegistration({ descr.serial, defaultCamBehaviour_.load() });

            cameraManager_->registerCamera(
                AravisGenICamCamera::create(descr.serial, descr.ifaceId)
            );
        }
    }
#endif

    // OpenCV
    {
        std::vector<CameraUtils::Descriptor> openCvDevices = CameraUtils::enumerateOpenCV();

        for (const auto& descr : openCvDevices)
        {
            addPendingRegistration({ descr.serial, defaultCamBehaviour_.load() });

            cameraManager_->registerCamera(
                OpenCVCamera::create(descr.serial, descr.ifaceId)
            );
        }
    }

    // Other interfaces...

    return true;
}

TaskResult CameraAssistantModel::discoverInterfaceTask(CameraInterface interfaceType)
{
    switch (interfaceType)
    {
#ifdef WITH_GENICAM
    case CameraInterface::ARAVIS_GENICAM:
    {
        std::vector<CameraUtils::Descriptor> arvDevices = CameraUtils::enumerateAravis();

        for (const auto& descr : arvDevices)
        {
            addPendingRegistration({ descr.serial, defaultCamBehaviour_.load() });

            cameraManager_->registerCamera(
                AravisGenICamCamera::create(descr.serial, descr.ifaceId)
            );
        }
        break;
    }
#endif

    case CameraInterface::OPENCV:
    {
        std::vector<CameraUtils::Descriptor> openCvDevices = CameraUtils::enumerateOpenCV();

        for (const auto& descr : openCvDevices)
        {
            addPendingRegistration({ descr.serial, defaultCamBehaviour_.load() });

            cameraManager_->registerCamera(
                OpenCVCamera::create(descr.serial, descr.ifaceId)
            );
        }

        break;
    }

    default:
        return {
            false,
            { "Interface not handled", Log::Level::LVL_ERROR }
        };
    }

    return true;
}

TaskResult CameraAssistantModel::loadCamerasConfigTask(const std::vector<CameraDTO>& cameraVec)
{
    if (cameraVec.empty())
    {
        return {
            false,
            { "No cameras provided", Log::Level::LVL_ERROR }
        };
    }

    std::vector<Log> exitLogs;

    for (const auto& cam : cameraVec)
    {
        if (cam.id.empty()) continue;

        std::shared_ptr<ICamera> iCam;
        switch (cam.camInterface)
        {
#ifdef WITH_GENICAM
        case CameraInterface::ARAVIS_GENICAM:
        {
            iCam = CameraUtils::createAravisCam(cam.id);

            if (!iCam)
            {
                exitLogs.push_back(
                    { "Could not find camera Id: " + cam.id, Log::Level::LVL_ERROR }
                );
            }
            break;
        }
#endif
        case CameraInterface::OPENCV:
        {
            iCam = CameraUtils::createOpenCVCamera(cam.id);

            if (!iCam)
            {
                exitLogs.push_back(
                    { "Could not find camera Id: " + cam.id, Log::Level::LVL_ERROR }
                );
            }
            break;
        }

        default:
            exitLogs.push_back(
                { "Interface not handled for camera Id: " + cam.id, Log::Level::LVL_ERROR }
            );
        }

        if (!iCam)
            continue;

        iCam->setParametersPersistence(cam.paramsPersistence);

        addPendingRegistration({ cam.id, cam.behaviour });

        // CameraManager will fail to register only if 'iCam' is nullptr
        cameraManager_->registerCamera(iCam);
    }

    return { true, std::move(exitLogs) };
}

void CameraAssistantModel::addPendingRegistration(CameraDescriptor reg)
{
    std::unique_lock lock(pendingRegistrationsMutex_);

    pendingRegistrations_[reg.id] = std::move(reg);
}

void CameraAssistantModel::removePendingRegistration(const std::string& cameraId)
{
    std::unique_lock lock(pendingRegistrationsMutex_);

    pendingRegistrations_.erase(cameraId);
}

void CameraAssistantModel::acceptPendingRegistration(const std::string& cameraId)
{
    CameraDescriptor pending;
    {
        std::unique_lock lock(pendingRegistrationsMutex_);

        auto it = pendingRegistrations_.find(cameraId);
        if (it == pendingRegistrations_.end())
            return;

        pending = std::move(it->second);

        pendingRegistrations_.erase(cameraId);
    }

    registerCamera(pending);
}

bool CameraAssistantModel::hasPendingRegistration(const std::string& cameraId)
{
    std::shared_lock lock(pendingRegistrationsMutex_);

    bool found = false;

    auto it = pendingRegistrations_.find(cameraId);
    if (it != pendingRegistrations_.end())
        found = true;

    return found;
}

void CameraAssistantModel::registerCamera(const CameraDescriptor& descr)
{
    std::shared_ptr<ICameraProxy> cam = cameraManager_->getCameraProxy(descr.id);
    if (!cam)
        return;

    {
        std::unique_lock lock(registeredCamerasMutex_);

        auto camIt = registeredCameras_.find(descr.id);
        if (camIt != registeredCameras_.end())
            return;

        registeredCameras_.emplace(descr.id, descr);

        bindCameraMessages(cam);
    }

    if (descr.behaviour >= CameraBehaviour::AUTO_CHECK)
    {
        cam->check();
    }
}

std::optional<CameraAssistantModel::CameraDescriptor> CameraAssistantModel::getCameraDescriptor(const std::string& cameraId)
{
    std::shared_lock lock(registeredCamerasMutex_);

    auto it = registeredCameras_.find(cameraId);
    if (it == registeredCameras_.end())
        return std::nullopt;

    return it->second;
}

void CameraAssistantModel::bindCameraMessages(const std::shared_ptr<ICameraProxy>& cam)
{
    if (!cam)
        return;

    cam->subscribe(MSG_CHECK_CAMERA, &CameraAssistantModel::onCameraCheck, this);
    cam->subscribe(MSG_CONNECT_CAMERA, &CameraAssistantModel::onCameraConnection, this);
    cam->subscribe(MSG_DISCONNECT_CAMERA, &CameraAssistantModel::onCameraDisconnection, this);
    cam->subscribe(MSG_START_GRABBING, &CameraAssistantModel::onCameraStartLive, this);
    cam->subscribe(MSG_STOP_GRABBING, &CameraAssistantModel::onCameraStopLive, this);
}

void CameraAssistantModel::bindCameraMessages(const std::shared_ptr<ICamera>& cam)
{
    if (!cam)
        return;

    cam->subscribe(MSG_CHECK_CAMERA, &CameraAssistantModel::onCameraCheck, this);
    cam->subscribe(MSG_CONNECT_CAMERA, &CameraAssistantModel::onCameraConnection, this);
    cam->subscribe(MSG_DISCONNECT_CAMERA, &CameraAssistantModel::onCameraDisconnection, this);
    cam->subscribe(MSG_START_GRABBING, &CameraAssistantModel::onCameraStartLive, this);
    cam->subscribe(MSG_STOP_GRABBING, &CameraAssistantModel::onCameraStopLive, this);
}

void CameraAssistantModel::doCameraCheck(const MessageTaskP<CameraState>& msg)
{
    const MessageTask::TaskStatus taskStatus = msg.status();

    if (taskStatus == MessageTask::TaskStatus::SUCCESS ||
        taskStatus == MessageTask::TaskStatus::FAIL)
    {
        MessageContext ctx;
        ctx.setAttribute(CAM_SERIAL, msg.context().getAttribute<std::string>(CAM_SERIAL));

        publish(MessageP<CameraState>(MSG_CAM_ASSIST_CHECK_CAMERA, msg.getPayload(), SourceId::none(), std::move(ctx)));
    }

    if (taskStatus == MessageTask::TaskStatus::SUCCESS)
    {
        const std::string& cameraId = msg.context().getAttribute<std::string>(CAM_SERIAL);

        std::optional<CameraAssistantModel::CameraDescriptor> descr = getCameraDescriptor(cameraId);
        std::shared_ptr<ICameraProxy> cam = cameraManager_->getCameraProxy(cameraId);

        if (cam && descr.has_value())
        {
            const CameraBehaviour behaviour = descr.value().behaviour;
            if (behaviour >= CameraBehaviour::AUTO_CONNECT)
            {
                cam->connect();
            }
        }
    }

    UpdateLogsMessage(msg);
}

void CameraAssistantModel::onCameraCheck(const MessageTaskP<CameraState>& msg)
{
    worker_.enqueueTask(&CameraAssistantModel::doCameraCheck, this, msg);
}

void CameraAssistantModel::doCameraConnection(const MessageTask& msg)
{
    const MessageTask::TaskStatus taskStatus = msg.status();

    if (taskStatus == MessageTask::TaskStatus::SUCCESS)
    {
        const std::string& cameraId = msg.context().getAttribute<std::string>(CAM_SERIAL);

        MessageContext ctx;
        ctx.setAttribute(CAM_SERIAL, cameraId);

        publish(Message(MSG_CAM_ASSIST_CONNECT_CAMERA_SUCCESS, SourceId::none(), std::move(ctx)));


        std::optional<CameraAssistantModel::CameraDescriptor> descr = getCameraDescriptor(cameraId);
        std::shared_ptr<ICameraProxy> cam = cameraManager_->getCameraProxy(cameraId);

        if (cam && descr.has_value())
        {
            const CameraBehaviour behaviour = descr.value().behaviour;
            if (behaviour >= CameraBehaviour::AUTO_LIVE)
            {
                cam->startGrab(5000);
            }
        }
    }
    else if (taskStatus == MessageTask::TaskStatus::FAIL)
    {
        MessageContext ctx;
        ctx.setAttribute(CAM_SERIAL, msg.context().getAttribute<std::string>(CAM_SERIAL));

        publish(Message(MSG_CAM_ASSIST_CONNECT_CAMERA_FAIL, SourceId::none(), std::move(ctx)));
    }

    UpdateLogsMessage(msg);
}

void CameraAssistantModel::onCameraConnection(const MessageTask& msg)
{
    worker_.enqueueTask(&CameraAssistantModel::doCameraConnection, this, msg);
}

void CameraAssistantModel::doCameraDisconnection(const MessageTask& msg)
{
    const MessageTask::TaskStatus taskStatus = msg.status();

    if (taskStatus == MessageTask::TaskStatus::SUCCESS ||
        taskStatus == MessageTask::TaskStatus::FAIL)
    {
        MessageContext ctx;
        ctx.setAttribute(CAM_SERIAL, msg.context().getAttribute<std::string>(CAM_SERIAL));

        publish(Message(MSG_CAM_ASSIST_DISCONNECT_CAMERA, SourceId::none(), std::move(ctx)));
    }

    UpdateLogsMessage(msg);
}

void CameraAssistantModel::onCameraDisconnection(const MessageTask& msg)
{
    worker_.enqueueTask(&CameraAssistantModel::doCameraDisconnection, this, msg);
}

void CameraAssistantModel::doCameraStartLive(const MessageTask& msg)
{
    const MessageTask::TaskStatus taskStatus = msg.status();

    if (taskStatus == MessageTask::TaskStatus::SUCCESS)
    {
        MessageContext ctx;
        ctx.setAttribute(CAM_SERIAL, msg.context().getAttribute<std::string>(CAM_SERIAL));

        publish(Message(MSG_CAM_ASSIST_CAM_START_LIVE, SourceId::none(), std::move(ctx)));
    }

    UpdateLogsMessage(msg);
}

void CameraAssistantModel::onCameraStartLive(const MessageTask& msg)
{
    worker_.enqueueTask(&CameraAssistantModel::doCameraStartLive, this, msg);
}

void CameraAssistantModel::doCameraStopLive(const MessageTask& msg)
{
    const MessageTask::TaskStatus taskStatus = msg.status();

    if (taskStatus == MessageTask::TaskStatus::SUCCESS ||
        taskStatus == MessageTask::TaskStatus::FAIL)
    {
        MessageContext ctx;
        ctx.setAttribute(CAM_SERIAL, msg.context().getAttribute<std::string>(CAM_SERIAL));

        publish(Message(MSG_CAM_ASSIST_CAM_STOP_LIVE, SourceId::none(), std::move(ctx)));
    }

    UpdateLogsMessage(msg);
}

void CameraAssistantModel::onCameraStopLive(const MessageTask& msg)
{
    worker_.enqueueTask(&CameraAssistantModel::doCameraStopLive, this, msg);
}

void CameraAssistantModel::doCameraTakeSnap(const MessageTask& msg)
{
    const MessageTask::TaskStatus taskStatus = msg.status();

    if (taskStatus == MessageTask::TaskStatus::SUCCESS)
    {
        MessageContext ctx;
        ctx.setAttribute(CAM_SERIAL, msg.context().getAttribute<std::string>(CAM_SERIAL));

        publish(Message(MSG_CAM_ASSIST_CAM_SNAP_SUCCESS, SourceId::none(), std::move(ctx)));
    }
    else if (taskStatus == MessageTask::TaskStatus::FAIL)
    {
        MessageContext ctx;
        ctx.setAttribute(CAM_SERIAL, msg.context().getAttribute<std::string>(CAM_SERIAL));

        publish(Message(MSG_CAM_ASSIST_CAM_SNAP_FAIL, SourceId::none(), std::move(ctx)));
    }

    UpdateLogsMessage(msg);
}

void CameraAssistantModel::onCameraTakeSnap(const MessageTask& msg)
{
    worker_.enqueueTask(&CameraAssistantModel::doCameraTakeSnap, this, msg);
}

void CameraAssistantModel::doRegisterCamera(const Message& msg)
{
    const std::string camId = msg.context().getAttribute<std::string>(CAM_SERIAL);

    if (hasPendingRegistration(camId))
    {
        acceptPendingRegistration(camId);
    }
    else
    {
        registerCamera({ camId, getDefaultCameraBehaviour()});
    }

    MessageContext ctx;
    ctx.setAttribute(CAM_SERIAL, camId);
    publish(Message(MSG_CAM_ASSIST_REGISTER_CAM, SourceId::none(), std::move(ctx)));

    getCameraDescriptor(camId);

    UpdateLogsMessage(msg);
}

void CameraAssistantModel::onRegisterCamera(const Message& msg)
{
    worker_.enqueueTask(&CameraAssistantModel::doRegisterCamera, this, msg);
}

void CameraAssistantModel::doUnregisterCamera(const Message& msg)
{
    const std::string camId = msg.context().getAttribute<std::string>(CAM_SERIAL);

    {
        std::unique_lock lock(registeredCamerasMutex_);

        registeredCameras_.erase(camId);
    }

    MessageContext ctx;
    ctx.setAttribute(CAM_SERIAL, camId);
    publish(Message(MSG_CAM_ASSIST_UNREGISTER_CAM, SourceId::none(), std::move(ctx)));

    UpdateLogsMessage(msg);
}

void CameraAssistantModel::onUnregisterCamera(const Message& msg)
{
    worker_.enqueueTask(&CameraAssistantModel::doUnregisterCamera, this, msg);
}
