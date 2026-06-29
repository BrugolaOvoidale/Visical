#include "CalibrationModel.hpp"
#include <UtilityFunctions.hpp>
#include <parameter/ParameterUtils.hpp>
#include <calibration/Board.hpp>
#include <calibration/calibrator/Calibrator.hpp>
#include <calibration/calibrator/CalibrationParametersRegistry.hpp>
#include <opencv2/calib3d.hpp>
#include "calibrated_board/CalibratedBoardPlugins.hpp"
#include "camera_model/CameraModelPlugins.hpp"
#include "../SharedCameraIntrinsics.hpp"
#include "../SharedCameraIntrinsicsMessages.hpp"


CalibrationModel::CalibrationModel(
    const std::shared_ptr<SharedCameraIntrinsics>& camIntrinsics,
    const std::shared_ptr<MessageLogger>& logger)
    : CalibrationStageModel(
        std::make_shared<Evaluator<CalibratedBoard>>(),
        std::make_shared<Evaluator<CameraModel>>(),
        logger)
{
    if (!camIntrinsics)
    {
        throw std::invalid_argument("camIntrinsics is nullptr");
    }

    camIntrinsics_ = camIntrinsics;

    init();
}

///////////////////////////////////////////////////////////////////////////////

std::shared_ptr<CalibrationModel> CalibrationModel::create(
    const std::shared_ptr<SharedCameraIntrinsics>& camIntrinsics,
    const std::shared_ptr<MessageLogger>& logger)
{
    struct Enabler : public CalibrationModel
    {
        Enabler(const std::shared_ptr<SharedCameraIntrinsics>& camIntrinsics,
            const std::shared_ptr<MessageLogger>& logger)
            : CalibrationModel(camIntrinsics, logger) {}
	};

    return std::make_shared<Enabler>(camIntrinsics, logger);
}

///////////////////////////////////////////////////////////////////////////////

void CalibrationModel::setWorkingDataset(const std::vector<std::shared_ptr<Board>>& boards)
{
    {
        std::unique_lock lock(workingDatasetMutex_);

        workingDataset_.clear();

        for (const auto& board : boards)
        {
            if (board->isDetected())
                workingDataset_.insert(board);
        }
    }

    {
        std::unique_lock lock(lastCalibResultMutex_);
        lastCalibResult_.reset();
    }
}

TaskEnqueueResult CalibrationModel::calibrateCamera()
{
    // TODO: is this ugly?
    if (countBoards() == 0)
    {
        return { TaskEnqueueResult::Status::REJECTED, { "No boards stored", Log::Level::LVL_ERROR } };
    }

    enqueueMessageTask(
        MSG_CALIBRATE_CAMERA,
        SourceId::none(),
        {},
        &CalibrationModel::calibrateCameraTask,
        this
    );

    return TaskEnqueueResult::Status::NO_ERRORS;
}

TaskResultP<CvImage> CalibrationModel::undistort(const CvImage& image) const
{
    std::optional<CalibrationResult> opt_calibRes = getLastCalibrationResult();
    if (!opt_calibRes.has_value())
        return { std::nullopt, "" };

    const CalibrationResult& calibRes = opt_calibRes.value();

    const cv::Mat optimalCamMatrix = cv::getOptimalNewCameraMatrix(
        calibRes.evaluatedCameraModel().object().cameraMatrix(),
        calibRes.evaluatedCameraModel().object().distortionModel().coeffs(),
        image.size(),
        1
    );


    cv::Mat undistortedMat;

    cv::undistort(
        image.mat(),
        undistortedMat,
        optimalCamMatrix,
        calibRes.evaluatedCameraModel().object().distortionModel().coeffs()
    );

    return CvImage(undistortedMat);
}

std::vector<std::shared_ptr<ParameterInfo>> CalibrationModel::getCalibrationParametersInfo()
{
    std::vector<std::shared_ptr<ParameterInfo>> vec;
    vec.reserve(14);    // worst case scenario
    vec.push_back(getRequiredParameter("solver_type", CalibrationParametersRegistry::CATEGORY));
    vec.push_back(getRequiredParameter("principal_point_mode", CalibrationParametersRegistry::CATEGORY));
    vec.push_back(getRequiredParameter("focal_length_mode", CalibrationParametersRegistry::CATEGORY));
    vec.push_back(getRequiredParameter("tangential_distortion_mode", CalibrationParametersRegistry::CATEGORY));
    vec.push_back(getRequiredParameter("use_initial_guess", CalibrationParametersRegistry::CATEGORY));

    vec.push_back(getRequiredParameter("fix_k1", CalibrationParametersRegistry::CATEGORY));
    vec.push_back(getRequiredParameter("fix_k2", CalibrationParametersRegistry::CATEGORY));
    vec.push_back(getRequiredParameter("fix_k3", CalibrationParametersRegistry::CATEGORY));



    std::shared_ptr<CameraIntrinsics> camIntrinsics = camIntrinsics_->get();

    const DistortionModel::Type distType = camIntrinsics->distortionModel().type();

    if (distType >= DistortionModel::Type::RATIONAL)
    {
        vec.push_back(getRequiredParameter("fix_k4", CalibrationParametersRegistry::CATEGORY));
        vec.push_back(getRequiredParameter("fix_k5", CalibrationParametersRegistry::CATEGORY));
        vec.push_back(getRequiredParameter("fix_k6", CalibrationParametersRegistry::CATEGORY));
    }
    
    if (distType >= DistortionModel::Type::THIN_PRISM)
    {
        vec.push_back(getRequiredParameter("fix_s1s2s3s4", CalibrationParametersRegistry::CATEGORY));
    }
    
    if (distType >= DistortionModel::Type::TILTED)
    {
        vec.push_back(getRequiredParameter("fix_tauxtauy", CalibrationParametersRegistry::CATEGORY));
    }

    return vec;
}

std::optional<std::shared_ptr<Board>> CalibrationModel::getBoard(BoardMap::Id id) const
{
    std::shared_lock lock(workingDatasetMutex_);

    if (!workingDataset_.contains(id))
		return std::nullopt;

    return workingDataset_.at(id);
}

std::map<BoardMap::Id, std::shared_ptr<Board>> CalibrationModel::getWorkingDataset() const
{
    std::shared_lock lock(workingDatasetMutex_);
    return workingDataset_.getAll();
}

size_t CalibrationModel::countBoards() const
{
    std::shared_lock lock(workingDatasetMutex_);
    return workingDataset_.size();
}

std::optional<BoardMap::Id> CalibrationModel::removeBoard(BoardMap::Id toRemoveId)
{
    std::optional<BoardMap::Id> nextIndex = std::nullopt;
    {
        std::unique_lock lock(workingDatasetMutex_);
        nextIndex = workingDataset_.remove(toRemoveId);
    }

    return nextIndex;
}

void CalibrationModel::clearDataset()
{
    std::unique_lock lock(workingDatasetMutex_);
    workingDataset_.clear();
}

std::optional<CalibrationResult> CalibrationModel::getLastCalibrationResult() const
{
    return lastCalibResult_;
}

///////////////////////////////////////////////////////////////////////////////

void CalibrationModel::init()
{
    boardEval_ = std::static_pointer_cast<Evaluator<CalibratedBoard>>(singleBoardEvaluator());

    boardSeqEval_ = std::static_pointer_cast<Evaluator<CameraModel>>(sequenceEvaluator());

    createDomainParameters();

    camIntrinsics_->subscribe(MSG_CAM_INTRINSICS_CHANGED, &CalibrationModel::onCamIntrinsicsChanged, this);

    validateCalibrationParameters();
}

void CalibrationModel::createDomainParameters()
{
    //
    CalibrationParametersRegistry calibParamsRegistry;

    initParameters(calibParamsRegistry.getAllParameters());
}

const std::unordered_map<std::string_view, std::function<std::shared_ptr<PluginContext>()>>& CalibrationModel::getBoardPluginsFactory() const
{
    return CalibratedBoardPlugins::factories();
}

const std::unordered_map<std::string_view, std::function<std::shared_ptr<PluginContext>()>>& CalibrationModel::getSequencePluginsFactory() const
{
    return CameraModelPlugins::factories();
}

void CalibrationModel::validateCalibrationParameters()
{
    const CameraIntrinsics::Mode mode = camIntrinsics_->get()->mode();

    switch (mode)
    {
        case CameraIntrinsics::Mode::AUTOMATIC:
        {
            getCurrentWritableParam("principal_point_mode", CalibrationParametersRegistry::CATEGORY)->setImposedAccessMode(
                AccessMode::READONLY,
                "Cannot fix principal point when camera intrinsics are automatic"
            );

            getCurrentWritableParam("focal_length_mode", CalibrationParametersRegistry::CATEGORY)->setImposedAccessMode(
                AccessMode::READONLY,
                "Cannot fix focal length when camera intrinsics are automatic"
            );

            getCurrentWritableParam("tangential_distortion_mode", CalibrationParametersRegistry::CATEGORY)->setImposedAccessMode(
                AccessMode::READONLY,
                "Cannot fix tangential distortion when camera intrinsics are automatic"
            );

            std::shared_ptr<Parameter> initialGuess = getCurrentWritableParam("use_initial_guess", CalibrationParametersRegistry::CATEGORY);
            initialGuess->resetImposedAccessMode();
            initialGuess->setValue<bool>(false);
            initialGuess->setImposedAccessMode(
                AccessMode::READONLY,
                "Cannot make an initial guess when camera intrinsics are automatic"
            );

            getCurrentWritableParam("tangential_distortion_mode", CalibrationParametersRegistry::CATEGORY)->setImposedAccessMode(
                AccessMode::READONLY,
                "Cannot fix tangential distortion when camera intrinsics are automatic"
            );

            getCurrentWritableParam("fix_k1", CalibrationParametersRegistry::CATEGORY)->setImposedAccessMode(
                AccessMode::READONLY,
                "Cannot fix k1 when camera intrinsics are automatic"
            );

            getCurrentWritableParam("fix_k2", CalibrationParametersRegistry::CATEGORY)->setImposedAccessMode(
                AccessMode::READONLY,
                "Cannot fix k2 when camera intrinsics are automatic"
            );

            getCurrentWritableParam("fix_k3", CalibrationParametersRegistry::CATEGORY)->setImposedAccessMode(
                AccessMode::READONLY,
                "Cannot fix k3 when camera intrinsics are automatic"
            );

            getCurrentWritableParam("fix_k4", CalibrationParametersRegistry::CATEGORY)->setImposedAccessMode(
                AccessMode::READONLY,
                "Cannot fix k4 when camera intrinsics are automatic"
            );

            getCurrentWritableParam("fix_k5", CalibrationParametersRegistry::CATEGORY)->setImposedAccessMode(
                AccessMode::READONLY,
                "Cannot fix k5 when camera intrinsics are automatic"
            );

            getCurrentWritableParam("fix_k6", CalibrationParametersRegistry::CATEGORY)->setImposedAccessMode(
                AccessMode::READONLY,
                "Cannot fix k6 when camera intrinsics are automatic"
            );

            getCurrentWritableParam("fix_s1s2s3s4", CalibrationParametersRegistry::CATEGORY)->setImposedAccessMode(
                AccessMode::READONLY,
                "Cannot fix s1s2s3s4 when camera intrinsics are automatic"
            );

            getCurrentWritableParam("fix_tauxtauy", CalibrationParametersRegistry::CATEGORY)->setImposedAccessMode(
                AccessMode::READONLY,
                "Cannot fix tauX and tauY when camera intrinsics are automatic"
            );

            break;
        }

        case CameraIntrinsics::Mode::FROM_HARDWARE:
        {
            getCurrentWritableParam("principal_point_mode", CalibrationParametersRegistry::CATEGORY)->resetImposedAccessMode();

            getCurrentWritableParam("focal_length_mode", CalibrationParametersRegistry::CATEGORY)->resetImposedAccessMode();

            getCurrentWritableParam("tangential_distortion_mode", CalibrationParametersRegistry::CATEGORY)->resetImposedAccessMode();

            std::shared_ptr<Parameter> initialGuess = getCurrentWritableParam("use_initial_guess", CalibrationParametersRegistry::CATEGORY);
            initialGuess->resetImposedAccessMode();
            initialGuess->setValue<bool>(true);
            initialGuess->setImposedAccessMode(
                AccessMode::READONLY,
                "Initial guess is required when camera intrinsics are from hardware"
            );

            getCurrentWritableParam("tangential_distortion_mode", CalibrationParametersRegistry::CATEGORY)->resetImposedAccessMode();

            getCurrentWritableParam("fix_k1", CalibrationParametersRegistry::CATEGORY)->resetImposedAccessMode();

            getCurrentWritableParam("fix_k2", CalibrationParametersRegistry::CATEGORY)->resetImposedAccessMode();

            getCurrentWritableParam("fix_k3", CalibrationParametersRegistry::CATEGORY)->resetImposedAccessMode();

            getCurrentWritableParam("fix_k4", CalibrationParametersRegistry::CATEGORY)->resetImposedAccessMode();

            getCurrentWritableParam("fix_k5", CalibrationParametersRegistry::CATEGORY)->resetImposedAccessMode();

            getCurrentWritableParam("fix_k6", CalibrationParametersRegistry::CATEGORY)->resetImposedAccessMode();

            getCurrentWritableParam("fix_s1s2s3s4", CalibrationParametersRegistry::CATEGORY)->resetImposedAccessMode();

            getCurrentWritableParam("fix_tauxtauy", CalibrationParametersRegistry::CATEGORY)->resetImposedAccessMode();

            break;
        }

        case CameraIntrinsics::Mode::FREE:
        {
            getCurrentWritableParam("principal_point_mode", CalibrationParametersRegistry::CATEGORY)->resetImposedAccessMode();

            getCurrentWritableParam("focal_length_mode", CalibrationParametersRegistry::CATEGORY)->resetImposedAccessMode();

            getCurrentWritableParam("tangential_distortion_mode", CalibrationParametersRegistry::CATEGORY)->resetImposedAccessMode();

            std::shared_ptr<Parameter> initialGuess = getCurrentWritableParam("use_initial_guess", CalibrationParametersRegistry::CATEGORY);
            initialGuess->resetImposedAccessMode();
            initialGuess->setValue<bool>(
                getDefaultParameter("use_initial_guess", CalibrationParametersRegistry::CATEGORY)->getValue<bool>()
            );

            getCurrentWritableParam("tangential_distortion_mode", CalibrationParametersRegistry::CATEGORY)->resetImposedAccessMode();

            getCurrentWritableParam("fix_k1", CalibrationParametersRegistry::CATEGORY)->resetImposedAccessMode();

            getCurrentWritableParam("fix_k2", CalibrationParametersRegistry::CATEGORY)->resetImposedAccessMode();

            getCurrentWritableParam("fix_k3", CalibrationParametersRegistry::CATEGORY)->resetImposedAccessMode();

            getCurrentWritableParam("fix_k4", CalibrationParametersRegistry::CATEGORY)->resetImposedAccessMode();

            getCurrentWritableParam("fix_k5", CalibrationParametersRegistry::CATEGORY)->resetImposedAccessMode();

            getCurrentWritableParam("fix_k6", CalibrationParametersRegistry::CATEGORY)->resetImposedAccessMode();

            getCurrentWritableParam("fix_s1s2s3s4", CalibrationParametersRegistry::CATEGORY)->resetImposedAccessMode();

            getCurrentWritableParam("fix_tauxtauy", CalibrationParametersRegistry::CATEGORY)->resetImposedAccessMode();

            break;
        }
    }

    publish(Message(MSG_CALIBRATION_PARAMETERS_CHANGED));
}

std::optional<CalibrationParameters> CalibrationModel::getCalibrationParameters()
{
    try
    {
        CalibrationParameters calibParams;

        calibParams.solver = UtilityFunctions::enumFromString<CalibrationParameters::SolverType>(getRequiredParameter("solver_type", CalibrationParametersRegistry::CATEGORY)->getValue<std::string>()
        ).value();

        calibParams.ppMode = UtilityFunctions::enumFromString<CalibrationParameters::PrincipalPointMode>(getRequiredParameter("principal_point_mode", CalibrationParametersRegistry::CATEGORY)->getValue<std::string>()
        ).value();

        calibParams.focalMode = UtilityFunctions::enumFromString<CalibrationParameters::FocalLengthMode>(getRequiredParameter("focal_length_mode", CalibrationParametersRegistry::CATEGORY)->getValue<std::string>()
        ).value();

        calibParams.tangentialMode = UtilityFunctions::enumFromString<CalibrationParameters::TangentialDistortionMode>(getRequiredParameter("tangential_distortion_mode", CalibrationParametersRegistry::CATEGORY)->getValue<std::string>()
        ).value();

        calibParams.useInitialGuess = getRequiredParameter("use_initial_guess", CalibrationParametersRegistry::CATEGORY)->getValue<bool>();
       
        calibParams.fixK1 = getRequiredParameter("fix_k1", CalibrationParametersRegistry::CATEGORY)->getValue<bool>();
        
        calibParams.fixK2 = getRequiredParameter("fix_k2", CalibrationParametersRegistry::CATEGORY)->getValue<bool>();
        
        calibParams.fixK3 = getRequiredParameter("fix_k3", CalibrationParametersRegistry::CATEGORY)->getValue<bool>();

        calibParams.fixK4 = getRequiredParameter("fix_k4", CalibrationParametersRegistry::CATEGORY)->getValue<bool>();

        calibParams.fixK5 = getRequiredParameter("fix_k5", CalibrationParametersRegistry::CATEGORY)->getValue<bool>();

        calibParams.fixK6 = getRequiredParameter("fix_k6", CalibrationParametersRegistry::CATEGORY)->getValue<bool>();

        calibParams.fixS1S2S3S4 = getRequiredParameter("fix_s1s2s3s4", CalibrationParametersRegistry::CATEGORY)->getValue<bool>();

        calibParams.fixTauXTauY = getRequiredParameter("fix_tauxtauy", CalibrationParametersRegistry::CATEGORY)->getValue<bool>();

        return calibParams;
    }
    catch (...)
    {
        return std::nullopt;
    }
}

std::optional<CalibrationResult> CalibrationModel::createCalibrationResult(
    const std::shared_ptr<CameraIntrinsics>& camIntrinsics,
    std::vector<EvaluatedCalibratedBoardEntry> evalCalibBoards,
    EvaluatedCameraModel evalCamModel) const
{
    const CameraModel& camModel = evalCamModel.object();

    // Calibrated camera matrix
    std::vector<std::shared_ptr<ParameterInfo>> cameraMatrixRes;

    // Calibrated horizontal focal length
    {
        std::shared_ptr<Parameter> tmp_aParam = ParameterUtils::cloneToParameter(
            camParamsRegistry_.getParameter("focal_length_x", CameraIntrinsicsRegistry::CATEGORY),
            AccessMode::READWRITE
        );

        double fx = camModel.cameraMatrix().at<double>(0, 0);
        tmp_aParam->setValue<double>(fx);

        std::shared_ptr<Parameter> aParam = ParameterUtils::clone(tmp_aParam, AccessMode::READONLY);

        cameraMatrixRes.push_back(aParam->getParameterView());
    }

    // Calibrated vertical focal length
    {
        std::shared_ptr<Parameter> tmp_aParam = ParameterUtils::cloneToParameter(
            camParamsRegistry_.getParameter("focal_length_y", CameraIntrinsicsRegistry::CATEGORY),
            AccessMode::READWRITE
        );

        double fy = camModel.cameraMatrix().at<double>(1, 1);
        tmp_aParam->setValue<double>(fy);

        std::shared_ptr<Parameter> aParam = ParameterUtils::clone(tmp_aParam, AccessMode::READONLY);

        cameraMatrixRes.push_back(aParam->getParameterView());
    }

    // Calibrated principal point X
    {
        std::shared_ptr<Parameter> tmp_aParam = ParameterUtils::cloneToParameter(
            camParamsRegistry_.getParameter("principal_point_x", CameraIntrinsicsRegistry::CATEGORY),
            AccessMode::READWRITE
        );

        double cx = camModel.cameraMatrix().at<double>(0, 2);
        tmp_aParam->setValue<double>(cx);

        std::shared_ptr<Parameter> aParam = ParameterUtils::clone(tmp_aParam, AccessMode::READONLY);

        cameraMatrixRes.push_back(aParam->getParameterView());
    }

    // Calibrated principal point Y
    {
        std::shared_ptr<Parameter> tmp_aParam = ParameterUtils::cloneToParameter(
            camParamsRegistry_.getParameter("principal_point_y", CameraIntrinsicsRegistry::CATEGORY),
            AccessMode::READWRITE
        );

        double cy = camModel.cameraMatrix().at<double>(1, 2);
        tmp_aParam->setValue<double>(cy);

        std::shared_ptr<Parameter> aParam = ParameterUtils::clone(tmp_aParam, AccessMode::READONLY);

        cameraMatrixRes.push_back(aParam->getParameterView());
    }


    // Calibrated distortion coefficients
    std::vector<std::shared_ptr<ParameterInfo>> distortionModelRes;

    // Calibrated distortion coefficients
    {
        auto createCoeff = [this](
            std::vector<std::shared_ptr<ParameterInfo>>& distortionModelRes,
            const std::string& name,
            const std::string& category,
            double value)
            {
                std::shared_ptr<Parameter> tmp_aParam = ParameterUtils::cloneToParameter(
                    distModelRegistry_.getParameter(name, category),
                    AccessMode::READWRITE
                );

                tmp_aParam->setValue<double>(value);

                std::shared_ptr<Parameter> aParam = ParameterUtils::clone(tmp_aParam, AccessMode::READONLY);

                distortionModelRes.push_back(aParam->getParameterView());
            };


        const cv::Mat& distCoeffs = camModel.distortionModel().coeffs();
        const DistortionModel::Type distModel = camIntrinsics->distortionModel().type();

        // Always present
        // Radial distortion
        createCoeff(
            distortionModelRes,
            "k1",
            DistortionModelRegistry::CATEGORY_STANDARD,
            distCoeffs.at<double>(0)
        );
        createCoeff(
            distortionModelRes,
            "k2",
            DistortionModelRegistry::CATEGORY_STANDARD,
            distCoeffs.at<double>(1)
        );
        createCoeff(
            distortionModelRes,
            "p1",
            DistortionModelRegistry::CATEGORY_STANDARD,
            distCoeffs.at<double>(2)
        );
        createCoeff(
            distortionModelRes,
            "p2",
            DistortionModelRegistry::CATEGORY_STANDARD,
            distCoeffs.at<double>(3)
        );
        createCoeff(
            distortionModelRes,
            "k3",
            DistortionModelRegistry::CATEGORY_STANDARD,
            distCoeffs.at<double>(4)
        );

        // Tangential distortion
        if (distModel >= DistortionModel::Type::RATIONAL)
        {
            createCoeff(
                distortionModelRes,
                "k4",
                DistortionModelRegistry::CATEGORY_RATIONAL,
                distCoeffs.at<double>(5)
            );
            createCoeff(
                distortionModelRes,
                "k5",
                DistortionModelRegistry::CATEGORY_RATIONAL,
                distCoeffs.at<double>(6)
            );
            createCoeff(
                distortionModelRes,
                "k6",
                DistortionModelRegistry::CATEGORY_RATIONAL,
                distCoeffs.at<double>(7)
            );
        }

        // Thin prism distortion
        if (distModel >= DistortionModel::Type::THIN_PRISM)
        {
            createCoeff(
                distortionModelRes,
                "s1",
                DistortionModelRegistry::CATEGORY_THIN_PRISM,
                distCoeffs.at<double>(8)
            );
            createCoeff(
                distortionModelRes,
                "s2",
                DistortionModelRegistry::CATEGORY_THIN_PRISM,
                distCoeffs.at<double>(9)
            );
            createCoeff(
                distortionModelRes,
                "s3",
                DistortionModelRegistry::CATEGORY_THIN_PRISM,
                distCoeffs.at<double>(10)
            );
            createCoeff(
                distortionModelRes,
                "s4",
                DistortionModelRegistry::CATEGORY_THIN_PRISM,
                distCoeffs.at<double>(11)
            );
        }

        // Tilt distortion
        if (distModel >= DistortionModel::Type::TILTED)
        {
            createCoeff(
                distortionModelRes,
                "tx",
                DistortionModelRegistry::CATEGORY_TILTED,
                distCoeffs.at<double>(12)
            );
            createCoeff(
                distortionModelRes,
                "ty",
                DistortionModelRegistry::CATEGORY_TILTED,
                distCoeffs.at<double>(13)
            );
        }
    }


    return CalibrationResult(
        std::move(cameraMatrixRes),
        std::move(distortionModelRes),
        std::move(evalCalibBoards),
        std::move(evalCamModel)
    );
}

TaskResultP<CalibrationResult> CalibrationModel::calibrateCameraTask()
{
    std::map<BoardMap::Id, std::shared_ptr<Board>> allBoards = getWorkingDataset();

    if (allBoards.empty())
    {
        return { std::nullopt, { "No detected boards stored", Log::Level::LVL_ERROR } };
    }

    std::shared_ptr<CameraIntrinsics> camIntrinsics = camIntrinsics_->get();

    std::optional<CalibrationParameters> calibParams = getCalibrationParameters();
    if (!calibParams.has_value())
    {
        return { std::nullopt, { "Could not create calibration parameters", Log::Level::LVL_ERROR } };
    }

    Calibrator calibrator(*camIntrinsics);
    calibrator.setCalibrationParameters(calibParams.value());

    std::vector<std::shared_ptr<Board>> workingDataset;
    workingDataset.reserve(allBoards.size());
    std::vector<BoardMap::Id> workingDatasetIds;
    workingDatasetIds.reserve(allBoards.size());
    for (const auto& entry : allBoards)
    {
		workingDatasetIds.push_back(entry.first);
        workingDataset.push_back(entry.second);
    }

    std::optional<CameraModel> opt_camModel;
    try
    {
        opt_camModel = calibrator.calibrateCamera(workingDataset);
    }
    catch (const std::exception& ex)
    {
        return { std::nullopt, { ex.what(), Log::Level::LVL_ERROR } };
    }

    CameraModel camModel = std::move(opt_camModel.value());

    const std::vector<CalibratedBoard> calibBoards = camModel.calibratedBoards();
    std::vector<EvaluatedCalibratedBoardEntry> evalCalibBoards;
    evalCalibBoards.reserve(calibBoards.size());
    for (size_t i = 0 ; i < allBoards.size() ; ++i)
    {
        evalCalibBoards.emplace_back(
            workingDatasetIds[i],
            boardEval_->evaluate(calibBoards[i])
        );
    }


    EvaluatedCameraModel evalCamModel = boardSeqEval_->evaluate(camModel);
    
    std::unique_lock lock(lastCalibResultMutex_);
    lastCalibResult_ = createCalibrationResult(
        camIntrinsics,
        std::move(evalCalibBoards),
        std::move(evalCamModel)
    );

    return lastCalibResult_;
}

TaskEnqueueResult CalibrationModel::reEvaluateAllBoards()
{
    if (countBoards() == 0)
        return TaskEnqueueResult::Status::ALREADY_DONE;

    worker_.enqueueTask(
        &CalibrationModel::doReEvaluateAllBoards,
        this
    );

    return TaskEnqueueResult::Status::NO_ERRORS;
}

TaskEnqueueResult CalibrationModel::reEvaluateAllBoards(const std::string& pluginId, bool enabled)
{
    if (countBoards() == 0)
        return TaskEnqueueResult::Status::ALREADY_DONE;

    worker_.enqueueTask(
        &CalibrationModel::doReEvaluateAllBoardsSingleCheck,
        this,
        pluginId,
        enabled
    );

    return TaskEnqueueResult::Status::NO_ERRORS;
}

TaskEnqueueResult CalibrationModel::reEvaluateSequence()
{
    worker_.enqueueTask(&CalibrationModel::doReEvaluateSequence, this);

    return TaskEnqueueResult::Status::NO_ERRORS;
}

TaskEnqueueResult CalibrationModel::reEvaluateSequence(const std::string& pluginId, bool enabled)
{
    worker_.enqueueTask(&CalibrationModel::doReEvaluateSequenceSingleCheck, this, pluginId, enabled);

    return TaskEnqueueResult::Status::NO_ERRORS;
}

void CalibrationModel::doReEvaluateAllBoards()
{
    std::vector<EvaluatedCalibratedBoardEntry> newEvalCalibBoards;

    std::optional<CalibrationResult> opt_lastCalibRes = getLastCalibrationResult();
    if (opt_lastCalibRes.has_value())
    {
        const CalibrationResult& lastCalibRes = opt_lastCalibRes.value();
        const std::vector<EvaluatedCalibratedBoardEntry>& oldEvalCalibBoards = lastCalibRes.evaluatedBoards();

        newEvalCalibBoards.reserve(oldEvalCalibBoards.size());

        for (const auto& entry : oldEvalCalibBoards)
        {
            newEvalCalibBoards.emplace_back(
                entry.mapId(),
                boardEval_->evaluate(entry.evaluatedCalibratedBoard().object())
            );
        }

        std::unique_lock lock(lastCalibResultMutex_);
        lastCalibResult_ = CalibrationResult(
            lastCalibRes.cameraMatrix(),
            lastCalibRes.distortionModel(),
            newEvalCalibBoards,
            lastCalibRes.evaluatedCameraModel()
        );
    }

    publish(MessageP<std::vector<EvaluatedCalibratedBoardEntry>>(MSG_CALIB_BOARD_REEVALUATION, std::move(newEvalCalibBoards)));
}

void CalibrationModel::doReEvaluateAllBoardsSingleCheck(
    const std::string& pluginId,
    bool enabled)
{
    std::vector<EvaluatedCalibratedBoardEntry> newEvalCalibBoards;

    std::optional<CalibrationResult> opt_lastCalibRes = getLastCalibrationResult();
    if (opt_lastCalibRes.has_value())
    {
        const CalibrationResult& lastCalibRes = opt_lastCalibRes.value();
        const std::vector<EvaluatedCalibratedBoardEntry>& oldEvalCalibBoards = lastCalibRes.evaluatedBoards();

        newEvalCalibBoards.reserve(oldEvalCalibBoards.size());

        for (const auto& entry : oldEvalCalibBoards)
        {
            if (enabled)
            {
                newEvalCalibBoards.emplace_back(
                    entry.mapId(),
                    boardEval_->updateEvaluation(entry.evaluatedCalibratedBoard(), pluginId)
                );
            }
            else
            {
                newEvalCalibBoards.emplace_back(
                    entry.mapId(),
                    boardEval_->updateEvaluation(entry.evaluatedCalibratedBoard(), pluginId)
                );
            }
        }

        std::unique_lock lock(lastCalibResultMutex_);
        lastCalibResult_ = CalibrationResult(
            lastCalibRes.cameraMatrix(),
            lastCalibRes.distortionModel(),
            newEvalCalibBoards,
            lastCalibRes.evaluatedCameraModel()
        );
    }

    publish(MessageP<std::vector<EvaluatedCalibratedBoardEntry>>(MSG_CALIB_BOARD_REEVALUATION, std::move(newEvalCalibBoards)));
}

void CalibrationModel::doReEvaluateSequence()
{
    std::optional<EvaluatedCameraModel> newEval;

    std::optional<CalibrationResult> opt_lastCalibRes = getLastCalibrationResult();
    if (opt_lastCalibRes.has_value())
    {
        const CalibrationResult& lastCalibRes = opt_lastCalibRes.value();

        newEval = boardSeqEval_->evaluate(lastCalibRes.evaluatedCameraModel().object());

        std::unique_lock lock(lastCalibResultMutex_);
        lastCalibResult_ = CalibrationResult(
            lastCalibRes.cameraMatrix(),
            lastCalibRes.distortionModel(),
            lastCalibRes.evaluatedBoards(),
            newEval.value()
        );
    }

    if (newEval.has_value())
    {
        publish(MessageP<EvaluatedCameraModel>(MSG_CAM_MODEL_REEVALUATION, std::move(newEval.value())));
    }
}

void CalibrationModel::doReEvaluateSequenceSingleCheck(
    const std::string& pluginId,
    bool enabled)
{
    std::optional<EvaluatedCameraModel> newEval;

    std::optional<CalibrationResult> opt_lastCalibRes = getLastCalibrationResult();
    if (opt_lastCalibRes.has_value())
    {
        const CalibrationResult& lastCalibRes = opt_lastCalibRes.value();

        if (enabled)
        {
            newEval = boardSeqEval_->updateEvaluation(lastCalibRes.evaluatedCameraModel(), pluginId);
        }
        else
        {
            newEval = boardSeqEval_->updateEvaluation(lastCalibRes.evaluatedCameraModel(), pluginId);
        }

        std::unique_lock lock(lastCalibResultMutex_);
        lastCalibResult_ = CalibrationResult(
            lastCalibRes.cameraMatrix(),
            lastCalibRes.distortionModel(),
            lastCalibRes.evaluatedBoards(),
            newEval.value()
        );
    }

    if (newEval.has_value())
    {
        publish(MessageP<EvaluatedCameraModel>(MSG_CAM_MODEL_REEVALUATION, std::move(newEval.value())));
    }
}

std::shared_ptr<const ParameterOwner> CalibrationModel::getSharedParameterOwner() const
{
    return std::static_pointer_cast<const ParameterOwner>(
        std::static_pointer_cast<const CalibrationModel>(shared_from_this())
    );
}

void CalibrationModel::onCamIntrinsicsChanged(const Message& msg)
{
    validateCalibrationParameters();
}