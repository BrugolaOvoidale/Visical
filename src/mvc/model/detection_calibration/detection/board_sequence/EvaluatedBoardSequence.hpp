#pragma once
#include <evaluator/EvaluationResult.hpp>


// Forward declarations
class Board;


using EvaluatedBoardSequence = EvaluationResult<std::vector<std::shared_ptr<Board>>>;
