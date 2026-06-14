#pragma once
#include <evaluator/EvaluationResult.hpp>
#include <calibration/calibrator/CameraModel.hpp>


/**
 * @brief Specialized EvaluationResult for CameraModel objects.
 *
 * This alias is used throughout the calibration module to represent the
 * final output of a calibrated CameraModel in the evaluation pipeline.
 */
using EvaluatedCameraModel = EvaluationResult<CameraModel>;