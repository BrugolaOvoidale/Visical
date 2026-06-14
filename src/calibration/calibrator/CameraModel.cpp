#include "CameraModel.hpp"


CameraModel::CameraModel(
	const cv::Mat& cameraMatrix,
	const DistortionModel& distModel,
	std::vector<CalibratedBoard> calibratedBoards,
	double reprojErr)
	: cameraMatrix_(cameraMatrix),
	distModel_(distModel),
	calibratedBoards_(std::move(calibratedBoards)),
	reprojErr_(reprojErr)
{
}

///////////////////////////////////////////////////

const cv::Mat& CameraModel::cameraMatrix() const
{
	return cameraMatrix_;
}

const DistortionModel& CameraModel::distortionModel() const
{
	return distModel_;
}

const std::vector<CalibratedBoard>& CameraModel::calibratedBoards() const
{
	return calibratedBoards_;
}

double CameraModel::reprojectionError() const
{
	return reprojErr_;
}