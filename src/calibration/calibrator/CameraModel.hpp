#pragma once
#include "CalibratedBoard.hpp"
#include "../DistortionModel.hpp"


/**
 * @brief Encapsulates the intrinsic parameters and calibration results for a camera.
 *
 * This class stores the camera matrix (intrinsics), the distortion coefficients
 * (via DistortionModel), and the metadata regarding the calibration process
 * (the calibrated boards results from the calibration).
 */
class CameraModel
{
public:
	//-----------------------------------------------------------------------------
	// Constructor/Destructor
	//-----------------------------------------------------------------------------

	/**
	 * @brief Constructs a CameraModel with complete calibration data.
	 * @param cameraMatrix The 3x3 intrinsic matrix.
	 * @param distModel The distortion coefficients and model type.
	 * @param calibratedBoards A collection of calibrated boards.
	 * @param reprojErr The final root-mean-square (RMS) reprojection error.
	 */
	CameraModel(
		const cv::Mat& cameraMatrix,
		const DistortionModel& distModel,
		std::vector<CalibratedBoard> calibratedBoards,
		double reprojErr
	);

	~CameraModel() = default;


	//-----------------------------------------------------------------------------
	// Class operators
	//-----------------------------------------------------------------------------

	/** @brief Returns the calibrated camera matrix */
	const cv::Mat& cameraMatrix() const;

	/** @brief Returns the calibrated distortion model */
	const DistortionModel& distortionModel() const;

	/** @brief Returns the list of calibrated boards. */
	const std::vector<CalibratedBoard>& calibratedBoards() const;

	/** @brief Returns the RMS reprojection error in pixels. */
	double reprojectionError() const;

private:
	// The intrinsic matrix
	cv::Mat cameraMatrix_;

	// The lens distortion parameters.
	DistortionModel distModel_;

	// Calibrated boards
	std::vector<CalibratedBoard> calibratedBoards_;

	// RMS reprojection error in pixels
	double reprojErr_;
};
