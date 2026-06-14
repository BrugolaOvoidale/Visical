#include "Calibrator.hpp"
#include <opencv2/calib3d.hpp>
#include "../Board.hpp"


Calibrator::Calibrator(CameraIntrinsics camIntrinsics)
    : camIntrinsics_(std::move(camIntrinsics))
{
}

//////////////////////////////////////////////////////

void Calibrator::setCameraIntrinsics(CameraIntrinsics camIntrinsics)
{
    camIntrinsics_ = std::move(camIntrinsics);
}

void Calibrator::setCalibrationParameters(CalibrationParameters calibParams)
{
    calibParams_ = std::move(calibParams);
}

CameraModel Calibrator::calibrateCamera(const std::vector<std::shared_ptr<Board>>& boards)
{
    validateCalibrationInput(boards);

    validateParameters();

	
	std::vector<std::vector<cv::Point3f>> objPoints;
	std::vector<std::vector<cv::Point2f>> imagePoints;

	for (const auto& b : boards)
	{
		objPoints.push_back(b->objectPoints());
		imagePoints.push_back(b->imagePoints());
	}

	
	cv::Mat cameraMatrix = camIntrinsics_.cameraMatrix().clone();
	cv::Mat distCoeffs = camIntrinsics_.distortionModel().coeffs().clone();
	std::vector<cv::Mat> rvecs, tvecs;

	double reprojErr = cv::calibrateCamera(
		objPoints,
		imagePoints,
		camIntrinsics_.imageSize(),
		cameraMatrix,
		distCoeffs,
		rvecs,
		tvecs,
		toOpenCVFlags()
	);

    std::vector<CalibratedBoard> calibratedBoards = generateCalibratedBoards(
        boards,
        objPoints,
        imagePoints,
        cameraMatrix,
        distCoeffs,
        rvecs,
        tvecs    
    );

	return CameraModel(
		std::move(cameraMatrix),
		std::move(distCoeffs),
		std::move(calibratedBoards),
		reprojErr
	);
}

///////////////////////////////////////////////////

// Convert to OpenCV flags
int Calibrator::toOpenCVFlags() const
{
    int flags = 0;

    // Intrinsic guess
    if (calibParams_.useInitialGuess)
    {
        flags |= cv::CALIB_USE_INTRINSIC_GUESS;
    }

    // Principal point
    if (calibParams_.ppMode == CalibrationParameters::PrincipalPointMode::FIX_AT_CENTER ||
        calibParams_.ppMode == CalibrationParameters::PrincipalPointMode::FIX_AT_INITIAL)
    {
        flags |= cv::CALIB_FIX_PRINCIPAL_POINT;
    }

    // Focal length
    switch (calibParams_.focalMode)
    {
        case CalibrationParameters::FocalLengthMode::FIX_ASPECT_RATIO:
            flags |= cv::CALIB_FIX_ASPECT_RATIO;
            break;

        case CalibrationParameters::FocalLengthMode::FIX_BOTH:
            flags |= cv::CALIB_FIX_FOCAL_LENGTH;
            break;

        case CalibrationParameters::FocalLengthMode::OPTIMIZE_BOTH:
            break;
    }

    // Tangential distortion
    switch (calibParams_.tangentialMode)
    {
    case CalibrationParameters::TangentialDistortionMode::FIX:
        flags |= cv::CALIB_FIX_TANGENT_DIST;
        break;

    case CalibrationParameters::TangentialDistortionMode::FORCE_ZERO:
        flags |= cv::CALIB_ZERO_TANGENT_DIST;
        break;

    case CalibrationParameters::TangentialDistortionMode::OPTIMIZE:
        break;
    }

    // Radial distortion coefficients
    if (calibParams_.fixK1) flags |= cv::CALIB_FIX_K1;
    if (calibParams_.fixK2) flags |= cv::CALIB_FIX_K2;
    if (calibParams_.fixK3) flags |= cv::CALIB_FIX_K3;
    if (calibParams_.fixK4) flags |= cv::CALIB_FIX_K4;
    if (calibParams_.fixK5) flags |= cv::CALIB_FIX_K5;
    if (calibParams_.fixK6) flags |= cv::CALIB_FIX_K6;

    // Distortion model
    switch (camIntrinsics_.distortionModel().type())
    {
        case DistortionModel::Type::RATIONAL:
            flags |= cv::CALIB_RATIONAL_MODEL;
            break;

        case DistortionModel::Type::THIN_PRISM:
            flags |= cv::CALIB_RATIONAL_MODEL;
            flags |= cv::CALIB_THIN_PRISM_MODEL;
            break;

        case DistortionModel::Type::TILTED:
            flags |= cv::CALIB_RATIONAL_MODEL;
            flags |= cv::CALIB_THIN_PRISM_MODEL;
            flags |= cv::CALIB_TILTED_MODEL;
            break;

        case DistortionModel::Type::STANDARD:
            break;
    }

    // Thin prism coefficients
    if (calibParams_.fixS1S2S3S4)
    {
        flags |= cv::CALIB_FIX_S1_S2_S3_S4;
    }

    // Tilted model coefficients
    if (calibParams_.fixTauXTauY)
    {
        flags |= cv::CALIB_FIX_TAUX_TAUY;
    }

    // Solver type
    switch (calibParams_.solver)
    {
        case CalibrationParameters::SolverType::QR:
            flags |= cv::CALIB_USE_QR;
            break;

        case CalibrationParameters::SolverType::LU:
            flags |= cv::CALIB_USE_LU;
            break;

        case CalibrationParameters::SolverType::SVD:
            break;
    }

    return flags;
}

void Calibrator::validateCalibrationInput(const std::vector<std::shared_ptr<Board>>& boards) const
{
    if (boards.empty())
    {
        throw std::invalid_argument("No boards provided");
    }

    for (size_t i = 0 ; i < boards.size() ; ++i)
    {
        if (!boards[i])
        {
            throw std::invalid_argument("Board at index " + std::to_string(i) + " is nullptr");
		}

        if (!boards[i]->isDetected())
        {
            throw std::invalid_argument("Board at index " + std::to_string(i) + " is not valid");
        }
    }
}

void Calibrator::validateParameters() const
{
	const DistortionModel::Type distModel = camIntrinsics_.distortionModel().type();

	// Validate distortion coefficient fixing
	if ((calibParams_.fixK4 || calibParams_.fixK5 || calibParams_.fixK6) &&
		distModel == DistortionModel::Type::STANDARD)
	{
	    throw std::logic_error(
	        "Cannot fix k4, k5, or k6 with STANDARD distortion model");
	}

	if (calibParams_.fixS1S2S3S4 &&
		distModel != DistortionModel::Type::THIN_PRISM &&
		distModel != DistortionModel::Type::TILTED)
	{
	    throw std::logic_error(
	        "Cannot fix thin prism coefficients without thin prism model");
	}

	if (calibParams_.fixTauXTauY &&
		distModel != DistortionModel::Type::TILTED)
	{
	    throw std::logic_error(
	        "Cannot fix tilted coefficients without tilted model");
	}
}

std::vector<CalibratedBoard> Calibrator::generateCalibratedBoards(
    const std::vector<std::shared_ptr<Board>>& boards,
    const std::vector<std::vector<cv::Point3f>>& objPoints,
    std::vector<std::vector<cv::Point2f>> imgPoints,
    const cv::Mat& camMatrix,
    const cv::Mat& distCoeffs,
    const std::vector<cv::Mat>& rvecs,
    const std::vector<cv::Mat>& tvecs) const
{
    std::vector<CalibratedBoard> calibratedBoards;
    calibratedBoards.reserve(rvecs.size());

    for (size_t i = 0; i < rvecs.size(); ++i)
    {
        std::vector<cv::Point2f> projectedPoints;
        const cv::Mat& currRvec = rvecs[i];
        const cv::Mat& currTvec = tvecs[i];
        const std::vector<cv::Point2f>& currImgPts = imgPoints[i];

        cv::projectPoints(
            objPoints[i],
            currRvec,
            currTvec,
            camMatrix,
            distCoeffs,
            projectedPoints
        );

        double sumSqErr = 0.0;
        double maxError = 0.0;

        size_t n = imgPoints.size();
        for (size_t j = 0; j < n; ++j)
        {
            cv::Point2f diff = currImgPts[j] - projectedPoints[j];
            double err = cv::norm(diff);

            sumSqErr += err * err;
            maxError = std::max(maxError, err);
        }

        double RMSerr = std::sqrt(sumSqErr / n); // RMS error
        calibratedBoards.push_back(
            CalibratedBoard(
                boards[i],
                currRvec,
                currTvec,
                RMSerr,
                maxError
            )
        );
    }

    return calibratedBoards;
}
