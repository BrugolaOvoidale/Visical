#include "DetectionModel.hpp"
#include <UtilityFunctions.hpp>
#include <parameter/ParameterUtils.hpp>
#include <camera_manager/CameraManager.hpp>
#include <camera_manager/ICameraProxy.hpp>
#include <calibration/Board.hpp>
#include <calibration/detector/Detector.hpp>
#include <calibration/detector/ChessboardParameters.hpp>
#include <calibration/detector/CircleboardParameters.hpp>
#include <calibration/detector/CharucoParameters.hpp>
#include <calibration/detector/AprilTagParameters.hpp>
#include <calibration/detector/PatternParametersRegistry.hpp>
#include <calibration/detector/ChessboardParametersRegistry.hpp>
#include <calibration/detector/CircleboardParametersRegistry.hpp>
#include <calibration/detector/ArucoParametersRegistry.hpp>
#include <calibration/detector/CharucoParametersRegistry.hpp>
#include <calibration/detector/AprilTagParametersRegistry.hpp>
#include "DetectionModelRegistry.hpp"
#include "board/BoardPlugins.hpp"
#include "board/EvaluatedBoard.hpp"
#include "board_sequence/BoardSequencePlugins.hpp"
#include "board_sequence/EvaluatedBoardSequence.hpp"
#include "../SharedCameraIntrinsics.hpp"


DetectionModel::DetectionModel(
    const std::shared_ptr<CameraManager>& cameraManager,
    const std::shared_ptr<SharedCameraIntrinsics>& camIntrinsics,
    const std::shared_ptr<MessageLogger>& logger)
    : CalibrationStageModel(
        std::make_shared<Evaluator<std::shared_ptr<Board>>>(),
        std::make_shared<Evaluator<std::vector<std::shared_ptr<Board>>>>(),
        logger)
{
    if (!cameraManager)
    {
        throw std::invalid_argument("cameraManager is nullptr");
    }

    if (!camIntrinsics)
    {
        throw std::invalid_argument("camIntrinsics is nullptr");
    }

    cameraManager_ = cameraManager;

    camIntrinsics_ = camIntrinsics;

    init();
}

///////////////////////////////////////////////////////////////////////////////

std::shared_ptr<DetectionModel> DetectionModel::create(
    const std::shared_ptr<CameraManager>& cameraManager,
    const std::shared_ptr<SharedCameraIntrinsics>& camIntrinsics,
    const std::shared_ptr<MessageLogger>& logger)
{
    struct Enabler : public DetectionModel
    {
        Enabler(
            const std::shared_ptr<CameraManager>& cameraManager,
            const std::shared_ptr<SharedCameraIntrinsics>& camIntrinsics,
            const std::shared_ptr<MessageLogger>& logger)
            : DetectionModel(cameraManager, camIntrinsics, logger) {}
	};

	return std::make_shared<Enabler>(cameraManager, camIntrinsics, logger);
}

///////////////////////////////////////////////////////////////////////////////

void DetectionModel::shutdown()
{
    stopSession();
}

TaskEnqueueResult DetectionModel::startFileSession(const std::vector<std::string>& filePaths)
{
    if (filePaths.empty())
    {
        return { TaskEnqueueResult::Status::REJECTED, { "Images path is empty", Log::Level::LVL_ERROR } };
    }

    if (isSessionOn())
    {
        return { TaskEnqueueResult::Status::ALREADY_ACTIVE, { "Session is already on", Log::Level::LVL_INFO } };
    }

    if (isStartingSession())
    {
        return { TaskEnqueueResult::Status::ALREADY_ACTIVE, { "Already trying to start a session", Log::Level::LVL_INFO } };
    }

    requestStartSession_.store(true);

    // If there the previous session was of a different type,
    // signal that a new session has been requested,
    // so this one will replace it
    const DetectionModel::SessionType prevSessionType = sessionType_.load();

    if (prevSessionType != SessionType::UNDEFINED && prevSessionType != SessionType::FILE)
    {
        clearSession();
    }

    sessionType_.store(SessionType::FILE);

    enqueueMessageTask(
        MSG_BOARD_FROM_IMAGE,
        SourceId::none(),
        {},
        &DetectionModel::doFindBoardFromImages,
        this,
        filePaths
    );

    return TaskEnqueueResult::Status::NO_ERRORS;
}

TaskEnqueueResult DetectionModel::startCameraSession(
    const std::string& cameraId,
    bool snap)
{
    if (cameraId.empty())
    {
        return { TaskEnqueueResult::Status::REJECTED, { "Camera id is empty", Log::Level::LVL_ERROR } };
    }

    if (isSessionOn())
    {
        return { TaskEnqueueResult::Status::ALREADY_ACTIVE, { "Session is already on", Log::Level::LVL_INFO } };
    }

    if (isStartingSession())
    {
        return { TaskEnqueueResult::Status::ALREADY_ACTIVE, { "Already trying to start a session", Log::Level::LVL_INFO } };
    }

    requestStartSession_.store(true);

    // If there the previous session was of a different type,
    // OR the working camera changed,
    // signal that a new session has been requested,
    // so this one will replace it
    const DetectionModel::SessionType prevSessionType = sessionType_.load();
    const std::optional<std::string> prevCamId = getWorkingCameraId();

    if ((prevSessionType != SessionType::UNDEFINED) &&
        (prevSessionType != SessionType::CAMERA || (prevCamId.has_value() && cameraId != prevCamId.value())))
    {
        clearSession();

        setWorkingCameraId(cameraId);
    }

    sessionType_.store(SessionType::CAMERA);

    TaskEnqueueResult res;

    if (snap)
    {
        isLiveSession_.store(false);

        res = takeSnapshot(cameraId);
    }
    else
    {
        isLiveSession_.store(true);

        res = startLive(cameraId);
    }

    if (res.getStatus() == TaskEnqueueResult::Status::REJECTED)
    {
        requestStartSession_.store(false);

        requestStopSession_.store(false);
    }

    return res;
}

TaskEnqueueResult DetectionModel::stopSession()
{
    // Check if a session is On
    if (!isSessionOn())
    {
        return { TaskEnqueueResult::Status::ALREADY_DONE, { "Session is not running", Log::Level::LVL_INFO } };
    }

    // "If it was already true, bail out. Otherwise, I now own the run."
    if (requestStopSession_.exchange(true))
    {
        // Already running, no-op
        return { TaskEnqueueResult::Status::ALREADY_ACTIVE, { "Already trying to stopping session", Log::Level::LVL_INFO } };
    }

    return TaskEnqueueResult::Status::NO_ERRORS;
}

bool DetectionModel::isSessionOn() const
{
    return isSessionOn_.load();
}

bool DetectionModel::isLiveSession() const
{
    return isLiveSession_.load();
}

bool DetectionModel::isStartingSession() const
{
    return requestStartSession_.load();
}

DetectionModel::SessionType DetectionModel::sessionType() const
{
    return sessionType_.load();
}

std::optional<std::string> DetectionModel::getWorkingCameraId() const
{
    std::shared_lock lock(currCameraIdMutex_);

    return currCameraId_;
}

std::optional<std::shared_ptr<DetectionResult>> DetectionModel::getDetectionResult(DetectionResultMap::Id id) const
{
    std::shared_lock lock(detectionsMutex_);

    if (!detections_.contains(id))
    {
        return std::nullopt;
	}

    return detections_.at(id);
}

std::optional<std::shared_ptr<DetectionResult>> DetectionModel::getDetectionResult(DetectionResult::Id id) const
{
    std::shared_lock lock(detectionsMutex_);

    if (!detections_.contains(id))
    {
        return std::nullopt;
    }

    return detections_.at(id);
}

std::optional<std::shared_ptr<DetectionResult>> DetectionModel::getLiveDetectionResult(DetectionResult::Id id) const
{
    std::shared_lock lock(liveResultsMutex_);

    if (!liveMap_.contains(id))
    {
        return std::nullopt;
    }

    return liveMap_.at(id);
}

std::shared_ptr<DetectionResult> DetectionModel::getLatestLiveDetectionResult() const
{
    std::shared_lock lock(liveResultsMutex_);

    if (liveQueue_.empty()) return nullptr;

    return liveQueue_.back();
}

std::map<DetectionResultMap::Id, std::shared_ptr<DetectionResult>> DetectionModel::getAllDetectionResults() const
{
    std::shared_lock lock(detectionsMutex_);
    return detections_.getAll();
}

std::vector<std::shared_ptr<EvaluatedBoard>> DetectionModel::getDetectedBoards() const
{
    std::map<DetectionResultMap::Id, std::shared_ptr<DetectionResult>> allRes = detections_.getAll();

    std::vector<std::shared_ptr<EvaluatedBoard>> evalBoards;

    std::shared_lock lock(detectionsMutex_);
    for (const auto& entry : allRes)
    {
        const std::shared_ptr<EvaluatedBoard>& e = entry.second->evaluatedBoard();
        if(e->object()->isDetected())
            evalBoards.push_back(e);
    }

    return evalBoards;
}

std::map<DetectionResultMap::Id, std::shared_ptr<const CvImage>> DetectionModel::getAllOriginalImages() const
{
    std::shared_lock lock(originalImagesMutex_);

    return originalImages_;
}

const std::shared_ptr<EvaluatedBoardSequence>& DetectionModel::getEvaluatedSequence()
{
    std::shared_lock lock(lastEvalSequenceMutex_);

    return lastEvalSequence_;
}

size_t DetectionModel::countDetectionResults() const
{
    std::shared_lock lock(detectionsMutex_);

    return detections_.size();
}

TaskResult DetectionModel::setParameter(
    const std::string& paramId,
    const std::string& categoryId,
    const std::string& value)
{
    TaskResult result = ParameterOwner::setParameter(paramId, categoryId, value);

    if (result.isSuccess())
    {
        if (paramId == "pattern_type" && categoryId == PatternParametersRegistry::CATEGORY())
        {
            publish(Message(MSG_PATTERN_TYPE_CHANGED));
        }
    }

    return result;
}

TaskResult DetectionModel::setParameter(
    const std::string& paramId,
    const std::string& categoryId,
    int value)
{
    TaskResult result = ParameterOwner::setParameter(paramId, categoryId, value);

    if (result.isSuccess())
    {
        if (paramId == "rotate")
        {
            rerunAllBoards();
        }
    }

    return result;
}

TaskResult DetectionModel::setParameter(
    const std::string& paramId,
    const std::string& categoryId,
    bool value)
{
    TaskResult result = ParameterOwner::setParameter(paramId, categoryId, value);

    if (result.isSuccess())
    {
        if (paramId == "mirror_x" || paramId == "mirror_y")
        {
            rerunAllBoards();
        }
    }

    return result;
}

TaskResult DetectionModel::resetParameter(
    const std::string& paramId,
    const std::string& categoryId)
{
    TaskResult result = ParameterOwner::resetParameter(paramId, categoryId);

    if (result.isSuccess())
    {
        if (paramId == "pattern_type" && categoryId == PatternParametersRegistry::CATEGORY())
        {
            publish(Message(MSG_PATTERN_TYPE_CHANGED));
        }
        else if (paramId == "rotate" || paramId == "mirror_x" || paramId == "mirror_y")
        {
            rerunAllBoards();
        }
    }

    return result;
}

TaskResult DetectionModel::resetDirtyParameters()
{
    TaskResult result = ParameterOwner::resetDirtyParameters();

    // TODO:
    // ...
    // publish(EVT_CAM_PARAM_CHANGED, Message());
    // ...

    return result;
}

std::optional<DetectionResultMap::Id> DetectionModel::removeDetectionResults(DetectionResultMap::Id toRemoveId)
{
    std::optional<DetectionResultMap::Id> nextId = std::nullopt;
    {
        std::unique_lock lock(detectionsMutex_);
        nextId = detections_.remove(toRemoveId);
    }

    {
        std::unique_lock lock(originalImagesMutex_);
        originalImages_.erase(toRemoveId);
    }

    // If board was succesfuly removed, evaluation sequence is now invalid
	if (nextId.has_value())
        isEvalSequenceInvalid_.store(true);

    reEvaluateSequence();

    return nextId;
}

void DetectionModel::removeAllDetectionsResults()
{
    {
        std::unique_lock lock(detectionsMutex_);
        detections_.clear();
    }

    {
        std::unique_lock lock(originalImagesMutex_);
        originalImages_.clear();
    }

    isEvalSequenceInvalid_.store(true);
    reEvaluateSequence();
}

std::optional<DetectionResultMap::Id> DetectionModel::promoteToSnapshot(DetectionResult::Id frameId)
{
    std::shared_ptr<DetectionResult> detRes;
    std::shared_ptr<const CvImage> origImg;
    {
        std::shared_lock lock(liveResultsMutex_);

        auto it = liveMap_.find(frameId);
        if (it != liveMap_.end())
        {
            detRes = it->second;
            origImg = liveOrigImgsMap_.at(frameId);
        }
    }

    if (!detRes)
    {
        return std::nullopt;
    }

    DetectionResultMap::Id newId = storeDetectionResult(detRes, origImg);

    reEvaluateSequence();

    return newId;
}

///////////////////////////////////////////////////////////////////////////////

void DetectionModel::init()
{
    boardEval_ = std::static_pointer_cast<Evaluator<std::shared_ptr<Board>>>(singleBoardEvaluator());

    boardSeqEval_ = std::static_pointer_cast<Evaluator<std::vector<std::shared_ptr<Board>>>>(sequenceEvaluator());

    createDomainParameters();
}

void DetectionModel::createDomainParameters()
{
    std::vector<std::shared_ptr<ParameterInfo>> infoVec;

    DetectionModelRegistry modelRegistry;
    UtilityFunctions::moveInto(modelRegistry.getAllParameters(), infoVec);

    PatternParametersRegistry baseDetParamsRegistry;
    UtilityFunctions::moveInto(baseDetParamsRegistry.getAllParameters(), infoVec);

    ChessboardParametersRegistry chessboardParamsRegistry;
    UtilityFunctions::moveInto(chessboardParamsRegistry.getAllParameters(), infoVec);

    CircleboardParametersRegistry circleBoardParamsRegistry;
    UtilityFunctions::moveInto(circleBoardParamsRegistry.getAllParameters(), infoVec);

    ArucoParametersRegistry arucoBoardParamsRegistry;
    UtilityFunctions::moveInto(arucoBoardParamsRegistry.getAllParameters(), infoVec);

    CharucoParametersRegistry charucoBoardParamsRegistry;
    UtilityFunctions::moveInto(charucoBoardParamsRegistry.getAllParameters(), infoVec);

    AprilTagParametersRegistry aprilTagParamsRegistry;
    UtilityFunctions::moveInto(aprilTagParamsRegistry.getAllParameters(), infoVec);

    initParameters(std::move(infoVec));
}

void DetectionModel::setWorkingCameraId(const std::string& cameraId)
{
    std::unique_lock lock(currCameraIdMutex_);

    currCameraId_ = cameraId;
}

void DetectionModel::clearSession()
{
    {
        std::unique_lock lock(detectionsMutex_);
        detections_.clear();
    }

    {
        std::unique_lock lock(originalImagesMutex_);
        originalImages_.clear();
    }

    {
        std::unique_lock lock(liveResultsMutex_);
        liveQueue_.clear();
        liveMap_.clear();

        liveOrigImgsQueue_.clear();
        liveOrigImgsMap_.clear();
    }

    {
        std::unique_lock lock(currCameraIdMutex_);
        currCameraId_.reset();
    }

    publish(Message(MSG_SESSION_CLEARED));

    reEvaluateSequence();
}

TaskEnqueueResult DetectionModel::startLive(const std::string& cameraId)
{
    std::shared_ptr<ICameraProxy> cam = cameraManager_->getCameraProxy(cameraId);
    if (!cam)
    {
        return { TaskEnqueueResult::Status::REJECTED, { "Camera " + cameraId + " not found in camera manager", Log::Level::LVL_ERROR } };
    }
    else if (!cam->isConnected())
    {
        return { TaskEnqueueResult::Status::REJECTED, { "Camera is not connected", Log::Level::LVL_ERROR } };
    }

    cam->subscribe(MSG_STOP_GRABBING, &DetectionModel::onCameraStopGrab, this);

    TaskEnqueueResult res;

    if (cam->isGrabbing())
    {
        res = findBoardFromLiveTask(cameraId);
    }
    else
    {
        cam->subscribe(MSG_START_GRABBING, &DetectionModel::onCameraStartGrab, this);

        res = cam->startGrab(5000);

        if (res.getStatus() == TaskEnqueueResult::Status::REJECTED)
        {
            cam->unsubscribe(MSG_START_GRABBING, &DetectionModel::onCameraStartGrab, this);

            cam->unsubscribe(MSG_STOP_GRABBING, &DetectionModel::onCameraStopGrab, this);
        }
    }

    return res;
}

TaskEnqueueResult DetectionModel::takeSnapshot(const std::string& cameraId)
{
    std::shared_ptr<ICameraProxy> cam = cameraManager_->getCameraProxy(cameraId);
    if (!cam)
    {
        return { TaskEnqueueResult::Status::REJECTED, { "Camera " + cameraId + " not found in camera manager", Log::Level::LVL_ERROR } };
    }
    else if (!cam->isConnected())
    {
        return { TaskEnqueueResult::Status::REJECTED, { "Camera is not connected", Log::Level::LVL_ERROR } };
    }
    else if (cam->isGrabbing())
    {
        return findBoardFromSnapTask(cam->getLatestFrame());
    }

    return cam->takeSnapshot(&DetectionModel::onCameraTakeSnap, this, 5000);
}

TaskEnqueueResult DetectionModel::findBoardFromSnapTask(const std::shared_ptr<const CvImage>& image)
{
    requestStartSession_.store(false);

    if (!image)
    {
        requestStopSession_.store(false);

        return { TaskEnqueueResult::Status::REJECTED, { "image is nullptr", Log::Level::LVL_ERROR } };
    }

    enqueueMessageTask(
        MSG_BOARD_FROM_SNAP,
        SourceId::none(),
        {},
        &DetectionModel::doFindBoardFromSnap,
        this,
        image
    );

    return TaskEnqueueResult::Status::NO_ERRORS;
}

TaskEnqueueResult DetectionModel::findBoardFromLiveTask(const std::string& cameraId)
{
    std::shared_ptr<ICameraProxy> cam = cameraManager_->getCameraProxy(cameraId);
    if (!cam)
    {
        return { TaskEnqueueResult::Status::REJECTED, { "Camera " + cameraId + " not found in camera manager", Log::Level::LVL_ERROR } };
    }

    TaskResultP<std::shared_ptr<PatternParameters>> detParamsRes = getDetectionParameters();

    if (!detParamsRes.isSuccess())
    {
        return { TaskEnqueueResult::Status::REJECTED, detParamsRes.takeLogs() };
    }

    std::shared_ptr<PatternParameters> detParams = detParamsRes.getPayload();

    std::optional<Detector> opt_detector;

    switch (detParams->patternType())
    {
        case BoardPattern::CHESSBOARD:
            opt_detector = Detector(*camIntrinsics_->get(), *std::static_pointer_cast<ChessboardParameters>(detParams));

            break;

        case BoardPattern::SYMMETRIC_CIRCLES:
        case BoardPattern::ASYMMETRIC_CIRCLES:
            opt_detector = Detector(*camIntrinsics_->get(), *std::static_pointer_cast<CircleboardParameters>(detParams));

            break;

        case BoardPattern::CHARUCO:
            opt_detector = Detector(*camIntrinsics_->get(), *std::static_pointer_cast<CharucoParameters>(detParams));

            break;

        case BoardPattern::APRIL_TAG:
            opt_detector = Detector(*camIntrinsics_->get(), *std::static_pointer_cast<AprilTagParameters>(detParams));

            break;

        default:
            return { TaskEnqueueResult::Status::REJECTED, { "Board pattern type not supported", Log::Level::LVL_ERROR } };
    }

    std::shared_ptr<Detector> detector = std::make_shared<Detector>(std::move(opt_detector.value()));

	// TODO: is this really the best way to do this?
    // Maybe 'MSG_BOARD_FROM_LIVE' should not be a MessageTask
    // and I should mimic 1:1 ICamera grab loop?

    publish(MessageTask::pending(MSG_BOARD_FROM_LIVE));

    worker_.enqueueTask([this, cam, detector]() {
        requestStartSession_.store(false);
        isSessionOn_.store(true);

        publish(MessageTask::started(MSG_BOARD_FROM_LIVE));

        scheduleNextLiveDetection(cam, detector);

        });

    return TaskEnqueueResult::Status::NO_ERRORS;
}

void DetectionModel::scheduleNextLiveDetection(
    const std::shared_ptr<ICameraProxy>& cam,
    const std::shared_ptr<Detector>& detector)
{
    if (!requestStopSession_.load())
    {
        worker_.enqueueTask(&DetectionModel::doFindBoardFromLive, this, cam, detector);
    }
    else
    {
        finalizeLiveDetectionLoop(cam);
    }
}

void DetectionModel::finalizeLiveDetectionLoop(const std::shared_ptr<ICameraProxy>& cam)
{
    TaskEnqueueResult stopCamGrab;
    if (cam->isGrabbing())
        stopCamGrab = cam->stopGrab();

    isSessionOn_.store(false);

    requestStopSession_.store(false);

    publish(MessageTask::success(MSG_BOARD_FROM_LIVE));

    UpdateLogs(stopCamGrab.takeLogs());
}

TaskEnqueueResult DetectionModel::rerunAllBoards()
{
    // TODO: is this ugly?
    if (countDetectionResults() == 0)
    {
        return TaskEnqueueResult::Status::ALREADY_DONE;
    }

    if (isRerunningBoard_.exchange(true))
    {
        rerunRequested_.store(true);
    }

    enqueueMessageTask(
        MSG_BOARD_REDETECTION,
        SourceId::none(),
        {},
        &DetectionModel::doRerunAllBoards,
        this
    );

    return TaskEnqueueResult::Status::NO_ERRORS;
}

TaskEnqueueResult DetectionModel::reEvaluateAllBoards()
{
    if (countDetectionResults() == 0)
        return TaskEnqueueResult::Status::ALREADY_DONE;

    enqueueMessageTask(
        MSG_BOARD_REEVALUATION,
        SourceId::none(),
        {},
        &DetectionModel::doReEvaluateAllBoards,
        this
    );

    return TaskEnqueueResult::Status::NO_ERRORS;
}

TaskEnqueueResult DetectionModel::reEvaluateAllBoards(
    const std::string& pluginId,
    bool enabled)
{
    if (countDetectionResults() == 0)
        return TaskEnqueueResult::Status::ALREADY_DONE;

    enqueueMessageTask(
        MSG_BOARD_REEVALUATION,
        SourceId::none(),
        {},
        &DetectionModel::doReEvaluateAllBoardsSingleCheck,
        this,
        pluginId,
        enabled
    );

    return TaskEnqueueResult::Status::NO_ERRORS;
}

TaskEnqueueResult DetectionModel::reEvaluateSequence()
{
    worker_.enqueueTask(&DetectionModel::doReEvaluateSequence, this);

    return TaskEnqueueResult::Status::NO_ERRORS;
}

TaskEnqueueResult DetectionModel::reEvaluateSequence(
    const std::string& pluginId,
    bool enabled)
{
    worker_.enqueueTask(&DetectionModel::doReEvaluateSequenceSingleCheck, this, pluginId, enabled);

    return TaskEnqueueResult::Status::NO_ERRORS;
}

TaskResult DetectionModel::doFindBoardFromImages(const std::vector<std::string>& filePaths)
{
    requestStartSession_.store(false);
    isSessionOn_.store(true);

    if (filePaths.empty())
    {
        isSessionOn_.store(false);

        requestStopSession_.store(false);

        return { false, { "Images path is empty", Log::Level::LVL_ERROR } };
    }

    TaskResultP<std::shared_ptr<PatternParameters>> detParamsRes = getDetectionParameters();

    if (!detParamsRes.isSuccess())
    {
        isSessionOn_.store(false);

        requestStopSession_.store(false);

        return { false, detParamsRes.takeLogs() };
    }

    std::shared_ptr<PatternParameters> detParams = detParamsRes.getPayload();

    std::optional<Detector> opt_detector;

    switch (detParams->patternType())
    {
    case BoardPattern::CHESSBOARD:
		opt_detector = Detector(*camIntrinsics_->get(), *std::static_pointer_cast<ChessboardParameters>(detParams));

        break;

    case BoardPattern::SYMMETRIC_CIRCLES:
    case BoardPattern::ASYMMETRIC_CIRCLES:
        opt_detector = Detector(*camIntrinsics_->get(), *std::static_pointer_cast<CircleboardParameters>(detParams));

        break;

    case BoardPattern::CHARUCO:
        opt_detector = Detector(*camIntrinsics_->get(), *std::static_pointer_cast<CharucoParameters>(detParams));

        break;

    case BoardPattern::APRIL_TAG:
        opt_detector = Detector(*camIntrinsics_->get(), *std::static_pointer_cast<AprilTagParameters>(detParams));

        break;

    default:
        isSessionOn_.store(false);
        requestStopSession_.store(false);

        return { false, { "Board pattern type not supported", Log::Level::LVL_ERROR } };
    }

    Detector detector = std::move(opt_detector.value());

    for (const auto& path : filePaths)
    {
        const std::shared_ptr<const CvImage> originalImage = std::make_shared<const CvImage>(path);

        CvImage transImage = preprocessImage(*originalImage);

        // Find calibration object

        std::shared_ptr<Board> board;
        
        try
        {
            board = detector.findBoard(transImage);
        }
        catch (const std::exception& ex)
        {
            UpdateLogs(Log("Error image '" + path + "' -> " + ex.what(), Log::Level::LVL_ERROR));

            continue;
        }

        std::shared_ptr<EvaluatedBoard> evalBoard = std::make_shared<EvaluatedBoard>(
            std::move(boardEval_->evaluate(board))
        );

        std::shared_ptr<DetectionResultFile> detRes = std::make_shared<DetectionResultFile>(evalBoard, path);

        if (requestStopSession_.exchange(false))
        {
            break;
        }
        else
        {
            storeDetectionResult(detRes, originalImage);
            
            doReEvaluateSequence();
        }
    }

    isSessionOn_.store(false);

    return true;
}

TaskResult DetectionModel::doFindBoardFromSnap(const std::shared_ptr<const CvImage>& originalImage)
{
    requestStartSession_.store(false);
    isSessionOn_.store(true);

    if (!originalImage)
    {
        isSessionOn_.store(false);

        requestStopSession_.store(false);

        return { false, { "Image is nullptr", Log::Level::LVL_ERROR } };
    }


    TaskResultP<std::shared_ptr<PatternParameters>> detParamsRes = getDetectionParameters();

    if (!detParamsRes.isSuccess())
    {
        isSessionOn_.store(false);

        requestStopSession_.store(false);

        return { false, detParamsRes.takeLogs() };
    }

    std::shared_ptr<PatternParameters> detParams = detParamsRes.getPayload();

    std::optional<Detector> opt_detector;

    switch (detParams->patternType())
    {
    case BoardPattern::CHESSBOARD:
        opt_detector = Detector(*camIntrinsics_->get(), *std::static_pointer_cast<ChessboardParameters>(detParams));

        break;

    case BoardPattern::SYMMETRIC_CIRCLES:
    case BoardPattern::ASYMMETRIC_CIRCLES:
        opt_detector = Detector(*camIntrinsics_->get(), *std::static_pointer_cast<CircleboardParameters>(detParams));

        break;

    case BoardPattern::CHARUCO:
        opt_detector = Detector(*camIntrinsics_->get(), *std::static_pointer_cast<CharucoParameters>(detParams));

        break;

    case BoardPattern::APRIL_TAG:
        opt_detector = Detector(*camIntrinsics_->get(), *std::static_pointer_cast<AprilTagParameters>(detParams));

        break;

    default:
        isSessionOn_.store(false);

        requestStopSession_.store(false);

        return { false, { "Board pattern type not supported", Log::Level::LVL_ERROR } };
    }

    Detector detector = std::move(opt_detector.value());

    // Find calibration object
    CvImage transImage = preprocessImage(*originalImage);

    std::shared_ptr<Board> board;

    try
    {
        board = detector.findBoard(transImage);
    }
    catch (const std::exception& ex)
    {
        isSessionOn_.store(false);

        requestStopSession_.store(false);

        return { false, Log(std::string("Error detection -> ") + ex.what(), Log::Level::LVL_ERROR) };
    }

    std::shared_ptr<EvaluatedBoard> evalBoard = std::make_shared<EvaluatedBoard>(
        std::move(boardEval_->evaluate(board))
    );

    std::shared_ptr<DetectionResult> detRes = std::make_shared<DetectionResultSnap>(evalBoard);

    if (!requestStopSession_.exchange(false))
    {
        storeDetectionResult(detRes, originalImage);

        doReEvaluateSequence();
    }

    isSessionOn_.store(false);

    return true;
}

void DetectionModel::doFindBoardFromLive(
    const std::shared_ptr<ICameraProxy>& cam,
    const std::shared_ptr<Detector>& detector)
{
    std::shared_ptr<const CvImage> originalImage = cam->getLatestFrame();
    if (!originalImage)
    {
		scheduleNextLiveDetection(cam, detector);

        return;
    }

    // Find calibration object

    CvImage transImage = preprocessImage(*originalImage);

    std::shared_ptr<Board> board;

    try
    {
        board = detector->findBoard(transImage);
    }
    catch (const std::exception& ex)
    {
        UpdateLogs(Log(std::string("Error image -> ") + ex.what(), Log::Level::LVL_ERROR));

        scheduleNextLiveDetection(cam, detector);

        return;
    }

    std::shared_ptr<EvaluatedBoard> evalBoard = std::make_shared<EvaluatedBoard>(
        std::move(boardEval_->evaluate(board))
    );

    if (!requestStopSession_.load())
    {
        storeLiveDetectionResult(
            std::make_shared<DetectionResultLive>(evalBoard),
            originalImage
        );
    }

    scheduleNextLiveDetection(cam, detector);
}

TaskResult DetectionModel::doRerunAllBoards()
{
    TaskResultP<std::shared_ptr<PatternParameters>> detParamsRes = getDetectionParameters();

    if (!detParamsRes.isSuccess())
    {
        return { false, detParamsRes.takeLogs() };
    }

    std::shared_ptr<PatternParameters> detParams = detParamsRes.getPayload();

    std::optional<Detector> opt_detector;

    switch (detParams->patternType())
    {
    case BoardPattern::CHESSBOARD:
		opt_detector = Detector(*camIntrinsics_->get(), *std::static_pointer_cast<ChessboardParameters>(detParams));

        break;

    case BoardPattern::SYMMETRIC_CIRCLES:
    case BoardPattern::ASYMMETRIC_CIRCLES:
        opt_detector = Detector(*camIntrinsics_->get(), *std::static_pointer_cast<CircleboardParameters>(detParams));

        break;

    case BoardPattern::CHARUCO:
        opt_detector = Detector(*camIntrinsics_->get(), *std::static_pointer_cast<CharucoParameters>(detParams));

        break;

    case BoardPattern::APRIL_TAG:
        opt_detector = Detector(*camIntrinsics_->get(), *std::static_pointer_cast<AprilTagParameters>(detParams));

        break;

    default:
        return { false, { "Board pattern type not supported", Log::Level::LVL_ERROR } };
    }

    Detector detector = std::move(opt_detector.value());


    auto local_origImages = getAllOriginalImages();
    for (const auto& [id, img] : local_origImages)
    {
        if (rerunRequested_.load())
        {
            rerunRequested_.store(false);

            break;
        }

        std::optional<std::shared_ptr<DetectionResult>> toUpdateBoardRes_opt = getDetectionResult(id);
        if (!toUpdateBoardRes_opt.has_value())
            continue;

        std::shared_ptr<DetectionResult> toUpdateBoardRes = toUpdateBoardRes_opt.value();

        CvImage transImage = preprocessImage(*img);

        // Find calibration board

        std::shared_ptr<Board> board;

        try
        {
            board = detector.findBoard(transImage);
        }
        catch (const std::exception& ex)
        {
            UpdateLogs(Log("Error image '" + std::to_string(id.get()) + "' -> " + ex.what(), Log::Level::LVL_ERROR));

            continue;
        }

        std::shared_ptr<EvaluatedBoard> evalBoard = std::make_shared<EvaluatedBoard>(
            std::move(boardEval_->evaluate(board))
        );


        std::shared_ptr<DetectionResult> newBoardRes;
        switch (toUpdateBoardRes->source())
        {
            case DetectionResult::Source::FILE:
                newBoardRes = std::make_shared<DetectionResultFile>(
                    evalBoard,
                    std::static_pointer_cast<DetectionResultFile>(toUpdateBoardRes)->imagePath()
                );

                break;

            case DetectionResult::Source::CAMERA_SNAPSHOT:
                newBoardRes = std::make_shared<DetectionResultSnap>(
                    evalBoard
                );

                break;

            case DetectionResult::Source::CAMERA_LIVE:
                newBoardRes = std::make_shared<DetectionResultSnap>(
                    evalBoard
                );

                break;
        }


        if (!updateDetectionResult(id, newBoardRes)) continue;   // Board was removed in the meantime

        doReEvaluateSequence();
    }

    isRerunningBoard_.store(false);

    return true;
}

TaskResult DetectionModel::doReEvaluateAllBoards()
{
    std::map<DetectionResultMap::Id, std::shared_ptr<DetectionResult>> local_boardResMap = getAllDetectionResults();
    for (const auto& entry : local_boardResMap)
    {
        const std::shared_ptr<DetectionResult>& res = entry.second;

        std::shared_ptr<EvaluatedBoard> newEvalBoard = std::make_shared<EvaluatedBoard>(
            std::move(boardEval_->evaluate(res->evaluatedBoard()->object()))
        );

        std::shared_ptr<DetectionResult> newBoardRes;
        switch (res->source())
        {
        case DetectionResult::Source::FILE:
            newBoardRes = std::make_shared<DetectionResultFile>(
                newEvalBoard,
                std::static_pointer_cast<DetectionResultFile>(res)->imagePath()
            );

            break;

        case DetectionResult::Source::CAMERA_SNAPSHOT:
            newBoardRes = std::make_shared<DetectionResultSnap>(
                newEvalBoard
            );

            break;

        case DetectionResult::Source::CAMERA_LIVE:
            newBoardRes = std::make_shared<DetectionResultSnap>(
                newEvalBoard
            );

            break;
        }

        if (!updateDetectionResult(entry.first, newBoardRes)) continue; // Board was removed in the meantime
    }

    return true;
}

TaskResult DetectionModel::doReEvaluateAllBoardsSingleCheck(
    const std::string& pluginId,
    bool enabled)
{
    std::map<DetectionResultMap::Id, std::shared_ptr<DetectionResult>> local_boardResMap = getAllDetectionResults();
    for (const auto& entry : local_boardResMap)
    {
        const std::shared_ptr<DetectionResult>& res = entry.second;

        std::shared_ptr<EvaluatedBoard> newEvalBoard;

        if (enabled)
        {
            newEvalBoard = std::make_shared<EvaluatedBoard>(
                boardEval_->updateEvaluation(*res->evaluatedBoard(), pluginId)
            );
        }
        else
        {
            newEvalBoard = std::make_shared<EvaluatedBoard>(
                boardEval_->updateEvaluation(*res->evaluatedBoard(), pluginId)
            );
        }

        std::shared_ptr<DetectionResult> newBoardRes;
        switch (res->source())
        {
        case DetectionResult::Source::FILE:
            newBoardRes = std::make_shared<DetectionResultFile>(
                newEvalBoard,
                std::static_pointer_cast<DetectionResultFile>(res)->imagePath()
            );

            break;

        case DetectionResult::Source::CAMERA_SNAPSHOT:
            newBoardRes = std::make_shared<DetectionResultSnap>(
                newEvalBoard
            );

            break;

        case DetectionResult::Source::CAMERA_LIVE:
            newBoardRes = std::make_shared<DetectionResultSnap>(
                newEvalBoard
            );

            break;
        }

        if (!updateDetectionResult(entry.first, newBoardRes)) continue; // Board was removed in the meantime
    }

    return true;
}

TaskResultP<std::shared_ptr<EvaluatedBoardSequence>> DetectionModel::doReEvaluateSequence()
{
    const std::vector<std::shared_ptr<EvaluatedBoard>> evalBoards = getDetectedBoards();
    std::vector<std::shared_ptr<Board>> detectedBoards;
    detectedBoards.reserve(evalBoards.size());
    for (const auto& e : evalBoards) detectedBoards.push_back(e->object());

    std::shared_ptr<EvaluatedBoardSequence> newEval = std::make_shared<EvaluatedBoardSequence>(
        std::move(boardSeqEval_->evaluate(detectedBoards))
    );

    storeLatestEvaluatedSequence(newEval);

    return newEval;
}

TaskResultP<std::shared_ptr<EvaluatedBoardSequence>> DetectionModel::doReEvaluateSequenceSingleCheck(
    const std::string& pluginId,
    bool enabled)
{
    std::shared_ptr<EvaluatedBoardSequence> newEval;

    if (!isEvalSequenceInvalid_.exchange(false))
    {
        std::shared_ptr<EvaluatedBoardSequence> oldEval = getEvaluatedSequence();
        if (oldEval)
        {
            if (enabled)
            {
                newEval = std::make_shared<EvaluatedBoardSequence>(
                    boardSeqEval_->updateEvaluation(*oldEval, pluginId)
                );
            }
            else
            {
                newEval = std::make_shared<EvaluatedBoardSequence>(
                    boardSeqEval_->updateEvaluation(*oldEval, pluginId)
                );
            }
        }
        else
        {
            const std::vector<std::shared_ptr<EvaluatedBoard>> evalBoards = getDetectedBoards();
            std::vector<std::shared_ptr<Board>> detectedBoards;
            detectedBoards.reserve(evalBoards.size());
            for (const auto& e : evalBoards) detectedBoards.push_back(e->object());

            std::shared_ptr<EvaluatedBoardSequence> newEval = std::make_shared<EvaluatedBoardSequence>(
                std::move(boardSeqEval_->evaluate(detectedBoards))
            );
        }
    }
    else
    {
        const std::vector<std::shared_ptr<EvaluatedBoard>> evalBoards = getDetectedBoards();
        std::vector<std::shared_ptr<Board>> detectedBoards;
        detectedBoards.reserve(evalBoards.size());
        for (const auto& e : evalBoards) detectedBoards.push_back(e->object());

        std::shared_ptr<EvaluatedBoardSequence> newEval = std::make_shared<EvaluatedBoardSequence>(
            std::move(boardSeqEval_->evaluate(detectedBoards))
        );
    }

    storeLatestEvaluatedSequence(newEval);

    return newEval;
}

CvImage DetectionModel::preprocessImage(const CvImage& input)
{
    const int rotDeg = getRequiredParameter("rotate", DetectionModelRegistry::CATEGORY_PRE_PROC)->getValue<int>();
    const bool mirrorX = getRequiredParameter("mirror_x", DetectionModelRegistry::CATEGORY_PRE_PROC)->getValue<bool>();
    const bool mirrorY = getRequiredParameter("mirror_y", DetectionModelRegistry::CATEGORY_PRE_PROC)->getValue<bool>();

    if (rotDeg == 0 && !mirrorX && !mirrorY)
    {
        return CvImage(input);
    }

    CvImage transformedImage = input.clone();  // Start with the input image

    if (rotDeg != 0)
    {
        try
        {
            transformedImage = transformedImage.rotate(static_cast<float>(rotDeg));
        }
        catch (const std::exception& ex)
        {
            UpdateLogs(Log(ex.what(), Log::Level::LVL_ERROR));
        }
    }

    if (mirrorX)
    {
        transformedImage = transformedImage.mirror(CvImage::FlipAxis::X);
    }

    if (mirrorY)
    {
        transformedImage = transformedImage.mirror(CvImage::FlipAxis::Y);
    }

    return transformedImage;
}

DetectionResultMap::Id DetectionModel::storeDetectionResult(
    const std::shared_ptr<DetectionResult>& result,
    const std::shared_ptr<const CvImage>& originalImage)
{
    DetectionResultMap::Id assignedId(0);
    {
        std::unique_lock lock(detectionsMutex_);

        assignedId = detections_.insert(result, result->id());
    }

    {
        std::unique_lock lock(originalImagesMutex_);

        originalImages_[assignedId] = originalImage;
    }

    publish(MessageP<DetectionResultMap::Entry>(MSG_BOARD_STORED, { assignedId, result }));

    return assignedId;
}

void DetectionModel::storeLatestEvaluatedSequence(const std::shared_ptr<EvaluatedBoardSequence>& evalSequence)
{
    std::unique_lock lock(lastEvalSequenceMutex_);

    lastEvalSequence_ = evalSequence;

    publish(MessageP<std::shared_ptr<EvaluatedBoardSequence>>(MSG_BOARD_SEQUENCE_REEVALUATED, evalSequence));
}

bool DetectionModel::updateDetectionResult(DetectionResultMap::Id id, const std::shared_ptr<DetectionResult>& board)
{
    std::unique_lock lock(detectionsMutex_);

    bool success = detections_.replace(id, board, board->id());

    if (success)
        publish(MessageP<DetectionResultMap::Entry>(MSG_BOARD_UPDATE, { id, board }));

    return success;
}

void DetectionModel::storeLiveDetectionResult(
    const std::shared_ptr<DetectionResult>& result,
    const std::shared_ptr<const CvImage>& originalImage)
{
    std::shared_ptr<DetectionResultLive> liveBoard = std::dynamic_pointer_cast<DetectionResultLive>(result);
    if (!liveBoard)
        return;


    // 1. Evict expired results
    const auto now = std::chrono::steady_clock::now();

    std::unique_lock lock(liveResultsMutex_);

    evictExpiredLiveDetectionResults(now);


    // 2. Insert new result
    liveQueue_.push_back(result);
    liveMap_[liveBoard->id()] = result;

    liveOrigImgsQueue_.push_back(originalImage);
    liveOrigImgsMap_[liveBoard->id()] = originalImage;
}

void DetectionModel::evictExpiredLiveDetectionResults(const std::chrono::steady_clock::time_point& now)
{
    while (!liveQueue_.empty())
    {
        const auto& front = liveQueue_.front();
        if (now - front->timestamp() <= liveRetention_)
            break;

        const DetectionResult::Id toEraseId = front->id();

        liveMap_.erase(
            toEraseId
        );

        liveQueue_.pop_front();

        liveOrigImgsMap_.erase(
            toEraseId
        );

        liveOrigImgsQueue_.pop_front();
    }
}

const std::unordered_map<std::string_view, std::function<std::shared_ptr<PluginContext>()>>& DetectionModel::getBoardPluginsFactory() const
{
    return BoardPlugins::factories();
}

const std::unordered_map<std::string_view, std::function<std::shared_ptr<PluginContext>()>>& DetectionModel::getSequencePluginsFactory() const
{
    return BoardSequencePlugins::factories();
}

TaskResultP<std::shared_ptr<PatternParameters>> DetectionModel::getDetectionParameters() const
{
    std::optional<std::shared_ptr<PatternParameters>> detParams;

    try
    {
        const BoardPattern pattern = UtilityFunctions::enumFromString<BoardPattern>(
            getRequiredParameter("pattern_type", PatternParametersRegistry::CATEGORY())->getValue<std::string>()
        ).value();
        const int patternCols = getRequiredParameter("pattern_cols", PatternParametersRegistry::CATEGORY())->getValue<int>();
        const int patternRows = getRequiredParameter("pattern_rows", PatternParametersRegistry::CATEGORY())->getValue<int>();

        switch (pattern)
        {
        case BoardPattern::CHESSBOARD:
        {
            ChessboardParameters::Geometry g(
                patternCols,
                patternRows,
                static_cast<float>(getRequiredParameter("square_size", ChessboardParametersRegistry::CATEGORY_GEOMETRY)->getValue<double>())
            );


            ChessboardParameters::Detection d;

            d.searchAccuracy = UtilityFunctions::enumFromString<ChessboardParameters::Detection::SearchAccuracy >(
                getRequiredParameter("search_accuracy", ChessboardParametersRegistry::CATEGORY_DETECTION)->getValue<std::string>()
            ).value();

            d.adaptiveThreshold = getRequiredParameter("adaptive_threshold", ChessboardParametersRegistry::CATEGORY_DETECTION)->getValue<bool>();

            d.normalizeImage = getRequiredParameter("normalize_image", ChessboardParametersRegistry::CATEGORY_DETECTION)->getValue<bool>();

            d.filterQuads = getRequiredParameter("filter_quads", ChessboardParametersRegistry::CATEGORY_DETECTION)->getValue<bool>();

            d.subpixelAccuracy = getRequiredParameter("subpixel_accuracy", ChessboardParametersRegistry::CATEGORY_DETECTION)->getValue<bool>();

            d.allowLargerBoards = getRequiredParameter("allow_larger_boards", ChessboardParametersRegistry::CATEGORY_DETECTION)->getValue<bool>();


            ChessboardParameters::Refine r;

            r.type = UtilityFunctions::enumFromString<ChessboardParameters::Refine::TermCriteriaType >(
                getRequiredParameter("term_criteria_type", ChessboardParametersRegistry::CATEGORY_REFINE)->getValue<std::string>()
            ).value();

            r.maxCount = getRequiredParameter("max_iterations", ChessboardParametersRegistry::CATEGORY_REFINE)->getValue<int>();

            r.epsilon = getRequiredParameter("epsilon", ChessboardParametersRegistry::CATEGORY_REFINE)->getValue<double>();

            std::shared_ptr<ChessboardParameters> chessParams = std::make_shared<ChessboardParameters>(std::move(g));

            chessParams->detection = std::move(d);

            chessParams->refine = std::move(r);

            detParams = chessParams;

            break;
        }

        case BoardPattern::SYMMETRIC_CIRCLES:
        case BoardPattern::ASYMMETRIC_CIRCLES:
        {
            CircleboardParameters::Geometry g(
                patternCols,
                patternRows,
                static_cast<float>(getRequiredParameter("mark_diameter", CircleboardParametersRegistry::CATEGORY_GEOMETRY)->getValue<double>()),
                static_cast<float>(getRequiredParameter("center_distance", CircleboardParametersRegistry::CATEGORY_GEOMETRY)->getValue<double>())
            );


            CircleboardParameters::Detection d;

            d.useClustering = getRequiredParameter("use_clustering", CircleboardParametersRegistry::CATEGORY_DETECTION)->getValue<bool>();

            std::shared_ptr<CircleboardParameters> circleParams = std::make_shared<CircleboardParameters>(
                std::move(g),
                pattern == BoardPattern::ASYMMETRIC_CIRCLES ? true : false
            );

            circleParams->detection = std::move(d);

            detParams = circleParams;

            break;
        }

        case BoardPattern::CHARUCO:
        {
            CharucoParameters::Geometry g({
                cv::Size(patternCols, patternRows),
                static_cast<float>(getRequiredParameter("square_size", CharucoParametersRegistry::CATEGORY_GEOMETRY)->getValue<double>()),
                static_cast<float>(getRequiredParameter("marker_length", ArucoParametersRegistry::CATEGORY_GEOMETRY())->getValue<double>()),
                cv::aruco::getPredefinedDictionary(getRequiredParameter("dictionary", CharucoParametersRegistry::CATEGORY_GEOMETRY)->getValue<int>())
                });

            CharucoParameters::Detection d;

            d.charuco.minMarkers = getRequiredParameter("min_markers", CharucoParametersRegistry::CATEGORY_DETECTION)->getValue<int>();
            d.charuco.tryRefineMarkers = getRequiredParameter("try_refine_markers", CharucoParametersRegistry::CATEGORY_DETECTION)->getValue<bool>();

            d.arucoDetector.adaptiveThreshWinSizeMin =
                getRequiredParameter("adaptive_thresh_win_size_min", ArucoParametersRegistry::CATEGORY_DETECTION())->getValue<int>();

            d.arucoDetector.adaptiveThreshWinSizeMax =
                getRequiredParameter("adaptive_thresh_win_size_max", ArucoParametersRegistry::CATEGORY_DETECTION())->getValue<int>();

            d.arucoDetector.adaptiveThreshWinSizeStep =
                getRequiredParameter("adaptive_thresh_win_size_step", ArucoParametersRegistry::CATEGORY_DETECTION())->getValue<int>();

            d.arucoDetector.adaptiveThreshConstant =
                getRequiredParameter("adaptive_thresh_constant", ArucoParametersRegistry::CATEGORY_DETECTION())->getValue<double>();

            d.arucoDetector.minMarkerPerimeterRate =
                getRequiredParameter("min_marker_perimeter_rate", ArucoParametersRegistry::CATEGORY_DETECTION())->getValue<double>();

            d.arucoDetector.maxMarkerPerimeterRate =
                getRequiredParameter("max_marker_perimeter_rate", ArucoParametersRegistry::CATEGORY_DETECTION())->getValue<double>();

            d.arucoDetector.polygonalApproxAccuracyRate =
                getRequiredParameter("polygonal_approx_accuracy_rate", ArucoParametersRegistry::CATEGORY_DETECTION())->getValue<double>();

            d.arucoDetector.minCornerDistanceRate =
                getRequiredParameter("min_corner_distance_rate", ArucoParametersRegistry::CATEGORY_DETECTION())->getValue<double>();

            d.arucoDetector.minDistanceToBorder =
                getRequiredParameter("min_distance_to_border", ArucoParametersRegistry::CATEGORY_DETECTION())->getValue<int>();

            d.arucoDetector.minMarkerDistanceRate =
                getRequiredParameter("min_marker_distance_rate", ArucoParametersRegistry::CATEGORY_DETECTION())->getValue<double>();

            d.arucoDetector.minGroupDistance =
                getRequiredParameter("min_group_distance", ArucoParametersRegistry::CATEGORY_DETECTION())->getValue<double>();

            d.arucoDetector.cornerRefinementMethod = getRequiredParameter("corner_refinement_method", CharucoParametersRegistry::CATEGORY_DETECTION)->getValue<int>();

            d.arucoDetector.cornerRefinementWinSize =
                getRequiredParameter("corner_refinement_win_size", ArucoParametersRegistry::CATEGORY_DETECTION())->getValue<int>();

            d.arucoDetector.relativeCornerRefinmentWinSize =
                getRequiredParameter("relative_corner_refinement_win_size", ArucoParametersRegistry::CATEGORY_DETECTION())->getValue<double>();

            d.arucoDetector.cornerRefinementMaxIterations =
                getRequiredParameter("corner_refinement_max_iterations", ArucoParametersRegistry::CATEGORY_DETECTION())->getValue<int>();

            d.arucoDetector.cornerRefinementMinAccuracy =
                getRequiredParameter("corner_refinement_min_accuracy", ArucoParametersRegistry::CATEGORY_DETECTION())->getValue<double>();

            d.arucoDetector.markerBorderBits =
                getRequiredParameter("marker_border_bits", ArucoParametersRegistry::CATEGORY_DETECTION())->getValue<int>();

            d.arucoDetector.perspectiveRemovePixelPerCell =
                getRequiredParameter("perspective_remove_pixel_per_cell", ArucoParametersRegistry::CATEGORY_DETECTION())->getValue<int>();

            d.arucoDetector.perspectiveRemoveIgnoredMarginPerCell =
                getRequiredParameter("perspective_remove_ignored_margin_per_cell", ArucoParametersRegistry::CATEGORY_DETECTION())->getValue<double>();

            d.arucoDetector.maxErroneousBitsInBorderRate =
                getRequiredParameter("max_erroneous_bits_in_border_rate", ArucoParametersRegistry::CATEGORY_DETECTION())->getValue<double>();

            d.arucoDetector.minOtsuStdDev =
                getRequiredParameter("min_otsu_std_dev", ArucoParametersRegistry::CATEGORY_DETECTION())->getValue<double>();

            d.arucoDetector.errorCorrectionRate =
                getRequiredParameter("error_correction_rate", ArucoParametersRegistry::CATEGORY_DETECTION())->getValue<double>();

            d.arucoDetector.detectInvertedMarker =
                getRequiredParameter("detect_inverted_marker", ArucoParametersRegistry::CATEGORY_DETECTION())->getValue<bool>();

            d.arucoDetector.useAruco3Detection =
                getRequiredParameter("use_aruco3_detection", ArucoParametersRegistry::CATEGORY_DETECTION())->getValue<bool>();

            d.arucoDetector.minSideLengthCanonicalImg =
                getRequiredParameter("min_side_length_canonical_img", ArucoParametersRegistry::CATEGORY_DETECTION())->getValue<int>();

            d.arucoDetector.minMarkerLengthRatioOriginalImg =
                getRequiredParameter("min_marker_length_ratio_original_img", ArucoParametersRegistry::CATEGORY_DETECTION())->getValue<double>();


            CharucoParameters::Refine r;

            r.arucoRefine.minRepDistance =
                getRequiredParameter("min_rep_distance", ArucoParametersRegistry::CATEGORY_REFINE())->getValue<double>();

            r.arucoRefine.errorCorrectionRate =
                getRequiredParameter("error_correction_rate", ArucoParametersRegistry::CATEGORY_REFINE())->getValue<double>();

            r.arucoRefine.checkAllOrders =
                getRequiredParameter("check_all_orders", ArucoParametersRegistry::CATEGORY_REFINE())->getValue<bool>();

            std::shared_ptr<CharucoParameters> charucoParams = std::make_shared<CharucoParameters>(std::move(g));

            charucoParams->detection = std::move(d);

            charucoParams->refine = std::move(r);

            detParams = charucoParams;

            break;
        }

        case BoardPattern::APRIL_TAG:
        {
            const cv::aruco::PredefinedDictionaryType dict = static_cast<cv::aruco::PredefinedDictionaryType>(getRequiredParameter("dictionary", AprilTagParametersRegistry::CATEGORY_GEOMETRY)->getValue<int>());

            AprilTagParameters::Geometry g({
                cv::Size(patternCols, patternRows),
                static_cast<float>(getRequiredParameter("marker_length", ArucoParametersRegistry::CATEGORY_GEOMETRY())->getValue<double>()),
                static_cast<float>(getRequiredParameter("marker_separation", AprilTagParametersRegistry::CATEGORY_GEOMETRY)->getValue<double>()),
                cv::aruco::getPredefinedDictionary(dict)
                },
                dict
            );

            ArucoParameters::Detection d;

            d.arucoDetector.adaptiveThreshWinSizeMin =
                getRequiredParameter("adaptive_thresh_win_size_min", ArucoParametersRegistry::CATEGORY_DETECTION())->getValue<int>();

            d.arucoDetector.adaptiveThreshWinSizeMax =
                getRequiredParameter("adaptive_thresh_win_size_max", ArucoParametersRegistry::CATEGORY_DETECTION())->getValue<int>();

            d.arucoDetector.adaptiveThreshWinSizeStep =
                getRequiredParameter("adaptive_thresh_win_size_step", ArucoParametersRegistry::CATEGORY_DETECTION())->getValue<int>();

            d.arucoDetector.adaptiveThreshConstant =
                getRequiredParameter("adaptive_thresh_constant", ArucoParametersRegistry::CATEGORY_DETECTION())->getValue<double>();

            d.arucoDetector.minMarkerPerimeterRate =
                getRequiredParameter("min_marker_perimeter_rate", ArucoParametersRegistry::CATEGORY_DETECTION())->getValue<double>();

            d.arucoDetector.maxMarkerPerimeterRate =
                getRequiredParameter("max_marker_perimeter_rate", ArucoParametersRegistry::CATEGORY_DETECTION())->getValue<double>();

            d.arucoDetector.polygonalApproxAccuracyRate =
                getRequiredParameter("polygonal_approx_accuracy_rate", ArucoParametersRegistry::CATEGORY_DETECTION())->getValue<double>();

            d.arucoDetector.minCornerDistanceRate =
                getRequiredParameter("min_corner_distance_rate", ArucoParametersRegistry::CATEGORY_DETECTION())->getValue<double>();

            d.arucoDetector.minDistanceToBorder =
                getRequiredParameter("min_distance_to_border", ArucoParametersRegistry::CATEGORY_DETECTION())->getValue<int>();

            d.arucoDetector.minMarkerDistanceRate =
                getRequiredParameter("min_marker_distance_rate", ArucoParametersRegistry::CATEGORY_DETECTION())->getValue<double>();

            d.arucoDetector.minGroupDistance =
                getRequiredParameter("min_group_distance", ArucoParametersRegistry::CATEGORY_DETECTION())->getValue<double>();

            d.arucoDetector.cornerRefinementMethod = getRequiredParameter("corner_refinement_method", AprilTagParametersRegistry::CATEGORY_DETECTION)->getValue<int>();

            d.arucoDetector.cornerRefinementWinSize =
                getRequiredParameter("corner_refinement_win_size", ArucoParametersRegistry::CATEGORY_DETECTION())->getValue<int>();

            d.arucoDetector.relativeCornerRefinmentWinSize =
                getRequiredParameter("relative_corner_refinement_win_size", ArucoParametersRegistry::CATEGORY_DETECTION())->getValue<double>();

            d.arucoDetector.cornerRefinementMaxIterations =
                getRequiredParameter("corner_refinement_max_iterations", ArucoParametersRegistry::CATEGORY_DETECTION())->getValue<int>();

            d.arucoDetector.cornerRefinementMinAccuracy =
                getRequiredParameter("corner_refinement_min_accuracy", ArucoParametersRegistry::CATEGORY_DETECTION())->getValue<double>();

            d.arucoDetector.markerBorderBits =
                getRequiredParameter("marker_border_bits", ArucoParametersRegistry::CATEGORY_DETECTION())->getValue<int>();

            d.arucoDetector.perspectiveRemovePixelPerCell =
                getRequiredParameter("perspective_remove_pixel_per_cell", ArucoParametersRegistry::CATEGORY_DETECTION())->getValue<int>();

            d.arucoDetector.perspectiveRemoveIgnoredMarginPerCell =
                getRequiredParameter("perspective_remove_ignored_margin_per_cell", ArucoParametersRegistry::CATEGORY_DETECTION())->getValue<double>();

            d.arucoDetector.maxErroneousBitsInBorderRate =
                getRequiredParameter("max_erroneous_bits_in_border_rate", ArucoParametersRegistry::CATEGORY_DETECTION())->getValue<double>();

            d.arucoDetector.minOtsuStdDev =
                getRequiredParameter("min_otsu_std_dev", ArucoParametersRegistry::CATEGORY_DETECTION())->getValue<double>();

            d.arucoDetector.errorCorrectionRate =
                getRequiredParameter("error_correction_rate", ArucoParametersRegistry::CATEGORY_DETECTION())->getValue<double>();

            d.arucoDetector.aprilTagQuadDecimate =
                getRequiredParameter("april_tag_quad_decimate", AprilTagParametersRegistry::CATEGORY_DETECTION)->getValue<double>();

            d.arucoDetector.aprilTagQuadSigma =
                getRequiredParameter("april_tag_quad_sigma", AprilTagParametersRegistry::CATEGORY_DETECTION)->getValue<double>();

            d.arucoDetector.aprilTagMinClusterPixels =
                getRequiredParameter("april_tag_min_cluster_pixels", AprilTagParametersRegistry::CATEGORY_DETECTION)->getValue<int>();

            d.arucoDetector.aprilTagMaxNmaxima =
                getRequiredParameter("april_tag_max_nmaxima", AprilTagParametersRegistry::CATEGORY_DETECTION)->getValue<int>();

            d.arucoDetector.aprilTagCriticalRad =
                getRequiredParameter("april_tag_critical_rad", AprilTagParametersRegistry::CATEGORY_DETECTION)->getValue<double>();

            d.arucoDetector.aprilTagMaxLineFitMse =
                getRequiredParameter("april_tag_max_line_fit_mse", AprilTagParametersRegistry::CATEGORY_DETECTION)->getValue<double>();

            d.arucoDetector.aprilTagMinWhiteBlackDiff =
                getRequiredParameter("april_tag_min_white_black_diff", AprilTagParametersRegistry::CATEGORY_DETECTION)->getValue<int>();

            d.arucoDetector.aprilTagDeglitch =
                getRequiredParameter("april_tag_deglitch", AprilTagParametersRegistry::CATEGORY_DETECTION)->getValue<int>();

            d.arucoDetector.detectInvertedMarker =
                getRequiredParameter("detect_inverted_marker", ArucoParametersRegistry::CATEGORY_DETECTION())->getValue<bool>();

            d.arucoDetector.useAruco3Detection =
                getRequiredParameter("use_aruco3_detection", ArucoParametersRegistry::CATEGORY_DETECTION())->getValue<bool>();

            d.arucoDetector.minSideLengthCanonicalImg =
                getRequiredParameter("min_side_length_canonical_img", ArucoParametersRegistry::CATEGORY_DETECTION())->getValue<int>();

            d.arucoDetector.minMarkerLengthRatioOriginalImg =
                getRequiredParameter("min_marker_length_ratio_original_img", ArucoParametersRegistry::CATEGORY_DETECTION())->getValue<double>();


            ArucoParameters::Refine r;

            r.arucoRefine.minRepDistance =
                getRequiredParameter("min_rep_distance", ArucoParametersRegistry::CATEGORY_REFINE())->getValue<double>();

            r.arucoRefine.errorCorrectionRate =
                getRequiredParameter("error_correction_rate", ArucoParametersRegistry::CATEGORY_REFINE())->getValue<double>();

            r.arucoRefine.checkAllOrders =
                getRequiredParameter("check_all_orders", ArucoParametersRegistry::CATEGORY_REFINE())->getValue<bool>();

            std::shared_ptr<AprilTagParameters> aprilTagParams = std::make_shared<AprilTagParameters>(std::move(g));

            aprilTagParams->detection = std::move(d);

            aprilTagParams->refine = std::move(r);

            detParams = aprilTagParams;

            break;
        }

        default:
            break;
        }

        if (!detParams.has_value())
        {
            return { std::nullopt, { "Unsupported board pattern", Log::Level::LVL_ERROR } };
        }
    }
    catch (const std::exception& ex)
    {
        return { std::nullopt, { ex.what(), Log::Level::LVL_ERROR}};
    }

    return detParams;
}

std::shared_ptr<const ParameterOwner> DetectionModel::getSharedParameterOwner() const
{
    return std::static_pointer_cast<const ParameterOwner>(
        std::static_pointer_cast<const DetectionModel>(shared_from_this())
    );
}

void DetectionModel::cameraStartGrab(const MessageTask& message)
{
    switch (message.status())
    {
    case MessageTask::TaskStatus::REJECTED:
    case MessageTask::TaskStatus::PENDING:
    case MessageTask::TaskStatus::STARTED:
        break;

    case MessageTask::TaskStatus::SUCCESS:
    {
        const std::string cameraId = message.context().getAttribute<std::string>(CAM_SERIAL);

        std::shared_ptr<ICameraProxy> cam = cameraManager_->getCameraProxy(cameraId);
        if (cam)
        {
            // Model should react to camera start grabbing ONLY if someone called 'findBoardFromLive()'
            // otherwise it means other parties started its acquisition, and Model should not care
            cam->unsubscribe(MSG_START_GRABBING, &DetectionModel::onCameraStartGrab, this);
            cam->unsubscribe(MSG_STOP_GRABBING, &DetectionModel::onCameraStopGrab, this);

            TaskEnqueueResult enqueueRes = findBoardFromLiveTask(cameraId);

            if (enqueueRes.getStatus() == TaskEnqueueResult::Status::REJECTED)
            {
                requestStartSession_.store(false);

                requestStopSession_.store(false);

                publish(MessageTask::rejected(MSG_BOARD_FROM_LIVE, enqueueRes.takeLogs()));
            }
        }
        else
        {
            requestStartSession_.store(false);

            requestStopSession_.store(false);

            publish(
                MessageTask::rejected(MSG_BOARD_FROM_LIVE, { Log{ "Camera " + cameraId + " not found in camera manager", Log::Level::LVL_ERROR } })
            );
        }

        break;
    }

    case MessageTask::TaskStatus::FAIL:
        requestStartSession_.store(false);

        requestStopSession_.store(false);

        const std::string cameraId = message.context().getAttribute<std::string>(CAM_SERIAL);

        std::shared_ptr<ICameraProxy> cam = cameraManager_->getCameraProxy(cameraId);
        if (cam)
        {
            cam->unsubscribe(MSG_START_GRABBING, &DetectionModel::onCameraStartGrab, this);
            cam->unsubscribe(MSG_STOP_GRABBING, &DetectionModel::onCameraStopGrab, this);
        }

        publish(
            MessageTask::rejected(MSG_BOARD_FROM_LIVE, { Log{ "Camera " + cameraId + " : failed to starting camera grab", Log::Level::LVL_ERROR } })
        );

        break;
    }

    //UpdateLogsMessage(message);
}

void DetectionModel::onCameraStartGrab(const MessageTask& message)
{
    worker_.enqueueTask(&DetectionModel::cameraStartGrab, this, message);
}

void DetectionModel::internalCameraStopGrab(const MessageTask& message)
{
    // Stop camera grabbing came from 'stopFindCalibBoardFromLive()':
    // stop Model live is already handled, so we just need to log the camera stopping

    switch (message.status())
    {
    case MessageTask::TaskStatus::REJECTED:
    case MessageTask::TaskStatus::PENDING:
    case MessageTask::TaskStatus::STARTED:
        break;

    case MessageTask::TaskStatus::SUCCESS:
    case MessageTask::TaskStatus::FAIL:
    {
        const std::string cameraId = message.context().getAttribute <std::string>(CAM_SERIAL);

        std::shared_ptr<ICameraProxy> cam = cameraManager_->getCameraProxy(cameraId);
        if (cam)
        {
            cam->unsubscribe(MSG_STOP_GRABBING, &DetectionModel::onCameraStopGrab, this);
        }

        break;
    }
    }

    UpdateLogsMessage(message);
}

void DetectionModel::externalCameraStopGrab(const MessageTask& message)
{
    // Some external module requested to stop camera grabbing:
    // tell the Model to stop its Live
    stopSession();    // Don't care about the output, nothing fatal could return

    switch (message.status())
    {
    case MessageTask::TaskStatus::REJECTED:
        break;

    case MessageTask::TaskStatus::PENDING:
    {
        const std::string cameraId = message.context().getAttribute <std::string>(CAM_SERIAL);

        std::shared_ptr<ICameraProxy> cam = cameraManager_->getCameraProxy(cameraId);
        if (cam)
        {
            // Model should react to camera start grabbing ONLY if someone called 'findBoardFromLive()'
            // otherwise it means other parties started its acquisition, and Model should not care
            cam->unsubscribe(MSG_START_GRABBING, &DetectionModel::onCameraStartGrab, this);
        }

        break;
    }

    case MessageTask::TaskStatus::STARTED:
        break;

    case MessageTask::TaskStatus::SUCCESS:
    case MessageTask::TaskStatus::FAIL:
    {
        const std::string cameraId = message.context().getAttribute <std::string>(CAM_SERIAL);

        std::shared_ptr<ICameraProxy> cam = cameraManager_->getCameraProxy(cameraId);
        if (cam)
        {
            cam->unsubscribe(MSG_STOP_GRABBING, &DetectionModel::onCameraStopGrab, this);
        }

        break;
    }
    }

    //UpdateLogsMessage(message);
}

void DetectionModel::onCameraStopGrab(const MessageTask& message)
{
    // If this stop request does come from
    if (requestStopSession_.load())
    {
		worker_.enqueueTask(&DetectionModel::internalCameraStopGrab, this, message);
    }
    else
    {
        worker_.enqueueTask(&DetectionModel::externalCameraStopGrab, this, message);
    }
}

void DetectionModel::cameraTakeSnap(const TaskResultP<std::shared_ptr<const CvImage>>& snapResult)
{
    if (snapResult.isSuccess())
    {
        TaskEnqueueResult enqueueRes = findBoardFromSnapTask(snapResult.getPayload());

        if (enqueueRes.getStatus() == TaskEnqueueResult::Status::REJECTED)
        {
            requestStartSession_.store(false);

            requestStopSession_.store(false);

            publish(MessageTask::rejected(MSG_BOARD_FROM_SNAP, enqueueRes.takeLogs()));
        }
    }
    else
    {
        requestStartSession_.store(false);

        requestStopSession_.store(false);

        publish(MessageTask::rejected(MSG_BOARD_FROM_SNAP, { Log{ "Failed to take camera snapshot", Log::Level::LVL_ERROR } }));
    }

    UpdateLogs(snapResult.getLogs());
}

void DetectionModel::onCameraTakeSnap(const TaskResultP<std::shared_ptr<const CvImage>>& snapResult)
{
	worker_.enqueueTask(&DetectionModel::cameraTakeSnap, this, snapResult);
}