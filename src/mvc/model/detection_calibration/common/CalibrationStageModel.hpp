#pragma once
#include <async_worker/AsyncWorker.hpp>
#include <parameter/ParameterOwner.hpp>
#include "../BoardEvaluationEngine.hpp"
#include "../../BaseModel.hpp"


class CalibrationStageModel :   public BaseModel,
                                public AsyncWorker,
                                public BoardEvaluationEngine,
                                public ParameterOwner
{
public:
    CalibrationStageModel(
        std::shared_ptr<IEvaluator> singleBoardEvaluator,
        std::shared_ptr<IEvaluator> boardSequenceEvaluator,
        const std::shared_ptr<MessageLogger>& logger = nullptr
    );

    ~CalibrationStageModel() = default;
};