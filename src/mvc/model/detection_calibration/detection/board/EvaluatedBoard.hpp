#pragma once
#include <evaluator/EvaluationResult.hpp>


// Forward declarations
class Board;


/**
 * @brief Specialized EvaluationResult for Board objects.
 *
 * This alias is used throughout the calibration module to represent the
 * final output of a detected board in the evaluation pipeline.
 */
using EvaluatedBoard = EvaluationResult<std::shared_ptr<Board>>;
