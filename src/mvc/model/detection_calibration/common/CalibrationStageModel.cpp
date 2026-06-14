#include "CalibrationStageModel.hpp"

CalibrationStageModel::CalibrationStageModel(
	std::shared_ptr<IEvaluator> singleBoardEvaluator,
	std::shared_ptr<IEvaluator> boardSequenceEvaluator,
	const std::shared_ptr<MessageLogger>& logger)
	: BaseModel(logger),
	BoardEvaluationEngine(
		std::move(singleBoardEvaluator),
		std::move(boardSequenceEvaluator))
{
}
