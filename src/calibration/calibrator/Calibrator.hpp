#pragma once
#include "CameraModel.hpp"
#include "CalibrationParameters.hpp"
#include "../CameraIntrinsics.hpp"


// Forward declarations
class Board;


/**
 * @brief Orchestrates the camera calibration process using calibration boards.
 *
 * This class consumes calibration boards and computes the intrinsic
 * and extrinsic camera parameters. It acts as a wrapper around sophisticated
 * optimization routines, converting high-level 'CalibrationParameters' into
 * specific solver flags and constraints.
 * You must call 'setCalibrationParameters' and 'setCameraParameters'
 * before attempting to run 'calibrateCamera'.
 */
class Calibrator
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    /**
     * @brief Construct the calibrator.
     * @param camIntrinsics Object containing the parameters of the camera.
     */
    Calibrator(CameraIntrinsics camIntrinsics);
    
    ~Calibrator() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /**
     * @brief Configures the camera intrinsic parameters.
     * @param camIntrinsics Object containing the parameters of the camera.
     */
    void setCameraIntrinsics(CameraIntrinsics camIntrinsics);

    /**
    * Set calibration parameters
    * @param calibParams CalibrationParameters
    */
    void setCalibrationParameters(CalibrationParameters calibParams);

    /**
    * Calibrate camera using previously detected boards
    * @param std::vector<Board> Input boards
    * @return CameraModel object containing calibration results
    * @throws If malformed input or invalid parameters combinations or OpenCV throws
    */
    CameraModel calibrateCamera(const std::vector<std::shared_ptr<Board>> &evalBoards);

private:
    /**
     * @brief Maps the internal CalibrationParameters to OpenCV-compatible bitmasks.
     * @return An integer representing the combined cv::CALIB_* flags.
     */
    int toOpenCVFlags() const;

    // Checks if the provided boards contain enough points for a valid solution.
    void validateCalibrationInput(const std::vector<std::shared_ptr<Board>>& boards) const;

    // Verifies that camera and calibration settings are logically consistent.
    void validateParameters() const;

    // Generate the calibrated boards from the calibration result
    std::vector<CalibratedBoard> generateCalibratedBoards(
        const std::vector<std::shared_ptr<Board>>& boards,
        const std::vector<std::vector<cv::Point3f>>& objPoints,
        std::vector<std::vector<cv::Point2f>> imgPoints,
        const cv::Mat& camMatrix,
        const cv::Mat& distCoeffs,
        const std::vector<cv::Mat>& rvecs,
        const std::vector<cv::Mat>& tvecs
    ) const;

private:
    // Camera parameters
    CameraIntrinsics camIntrinsics_;

    // Calibration parameters
    CalibrationParameters calibParams_;
};
