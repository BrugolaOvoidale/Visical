#pragma once
#include "camera_model/EvaluatedCameraModel.hpp"
#include "EvaluatedCalibratedBoardEntry.hpp"


// Forward declarations
class ParameterInfo;


/**
 * @brief Container for the output of a camera calibration process.
 *
 * This class stores the optimized camera intrinsic and distortion
 * coefficients parameters, and the evaluation metrics for both the
 * individual calibration boards and the global camera model.
 */
class CalibrationResult
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    /**
     * @brief Constructs a complete CalibrationResult.
     *
     * @param cameraMatrixRes Optimized intrinsic parameters.
     * @param distortionModelRes Optimized distortion coefficients.
     * @param evalCalibBoards Evaluation data for each individual board used in calibration.
     * @param evalCamModel Overall geometric evaluation of the final camera model.
     */
    CalibrationResult(
        std::vector<std::shared_ptr<ParameterInfo>> cameraMatrixRes,
        std::vector<std::shared_ptr<ParameterInfo>> distortionModelRes,
        std::vector<EvaluatedCalibratedBoardEntry> evalCalibBoards,
        EvaluatedCameraModel evalCamModel
    );

    ~CalibrationResult() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------
     
    /** @brief Returns the camera intrinsic matrix parameters. */
    const std::vector<std::shared_ptr<ParameterInfo>>& cameraMatrix() const;

    /** @brief Returns the distortion model parameters. */
    const std::vector<std::shared_ptr<ParameterInfo>>& distortionModel() const;

    /** @brief Returns evaluation metrics for all calibration boards.  */
    const std::vector<EvaluatedCalibratedBoardEntry>& evaluatedBoards() const;

    /** @brief Returns the overall evaluation of the camera model. */
    const EvaluatedCameraModel& evaluatedCameraModel() const;

private:
    // Intrinsic parameter results.
    std::vector<std::shared_ptr<ParameterInfo>> cameraMatrixRes_;

    // Distortion parameter results
    std::vector<std::shared_ptr<ParameterInfo>> distortionModelRes_;

    // Per-board evaluation data.
    std::vector<EvaluatedCalibratedBoardEntry> evalCalibBoards_;

    // Global model evaluation.
    EvaluatedCameraModel evalCamModel_;
};