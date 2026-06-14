#include "EvaluatedCalibratedBoardEntry.hpp"


EvaluatedCalibratedBoardEntry::EvaluatedCalibratedBoardEntry(
    BoardMap::Id mapId,
    EvaluatedCalibratedBoard evalCalibBoard)
    : mapId_(mapId),
    evalCalibBoard_(std::move(evalCalibBoard))
{
}

//////////////////////////////////////////////////

BoardMap::Id EvaluatedCalibratedBoardEntry::mapId() const
{
    return mapId_;
}

const EvaluatedCalibratedBoard& EvaluatedCalibratedBoardEntry::evaluatedCalibratedBoard() const
{
    return evalCalibBoard_;
}