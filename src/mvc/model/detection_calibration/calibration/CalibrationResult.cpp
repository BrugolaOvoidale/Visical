#include "CalibrationResult.hpp"


CalibrationResult::CalibrationResult(
	std::vector<std::shared_ptr<ParameterInfo>> cameraMatrixRes,
	std::vector<std::shared_ptr<ParameterInfo>> distortionModelRes,
	std::vector<EvaluatedCalibratedBoardEntry> evalCalibBoards,
	EvaluatedCameraModel evalCamModel)
	: cameraMatrixRes_(std::move(cameraMatrixRes)),
	distortionModelRes_(std::move(distortionModelRes)),
	evalCalibBoards_(std::move(evalCalibBoards)),
	evalCamModel_(std::move(evalCamModel))
{
}

////////////////////////////////////////

const std::vector<std::shared_ptr<ParameterInfo>>& CalibrationResult::cameraMatrix() const
{
    return cameraMatrixRes_;
}

const std::vector<std::shared_ptr<ParameterInfo>>& CalibrationResult::distortionModel() const
{
    return distortionModelRes_;
}

const std::vector<EvaluatedCalibratedBoardEntry>& CalibrationResult::evaluatedBoards() const
{
    return evalCalibBoards_;
}

const EvaluatedCameraModel& CalibrationResult::evaluatedCameraModel() const
{
    return evalCamModel_;
}