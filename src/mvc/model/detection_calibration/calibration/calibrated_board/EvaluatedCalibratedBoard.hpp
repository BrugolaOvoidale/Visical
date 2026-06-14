#pragma once
#include <evaluator/EvaluationResult.hpp>
#include <calibration/calibrator/CalibratedBoard.hpp>


/**
 * @brief Specialized EvaluationResult for CalibratedBoard objects.
 *
 * This alias is used throughout the calibration module to represent the
 * final output of a calibrated board in the evaluation pipeline.
 */
using EvaluatedCalibratedBoard = EvaluationResult<CalibratedBoard>;