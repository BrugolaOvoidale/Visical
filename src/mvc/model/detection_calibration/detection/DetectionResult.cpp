#include "DetectionResult.hpp"
#include "board/EvaluatedBoard.hpp"


DetectionResult::DetectionResult(
    Source source,
    const std::shared_ptr<EvaluatedBoard>& evalBoard)
    : boardSource_(source),
    evalBoard_(evalBoard),
    timestamp_(std::chrono::steady_clock::now()),
    id_(++idCounter)
{
}

/////////////////////////////////////////////////////

DetectionResult::Source DetectionResult::source() const
{
    return boardSource_;
}

const std::shared_ptr<EvaluatedBoard>& DetectionResult::evaluatedBoard() const
{
    return evalBoard_;
}

const std::chrono::steady_clock::time_point& DetectionResult::timestamp() const
{ 
    return timestamp_;
}

DetectionResult::Id DetectionResult::id() const
{ 
    return id_;
}


/////////////////////////////////////////////////////
/////////////////////////////////////////////////////


DetectionResultFile::DetectionResultFile(
    const std::shared_ptr<EvaluatedBoard>& evalBoard,
    const std::string& imagePath)
    : DetectionResult(Source::FILE, evalBoard), imagePath_(imagePath)
{
}

/////////////////////////////////////////////////////

const std::string& DetectionResultFile::imagePath() const
{
    return imagePath_;
}


/////////////////////////////////////////////////////
/////////////////////////////////////////////////////


DetectionResultSnap::DetectionResultSnap(const std::shared_ptr<EvaluatedBoard>& evalBoard)
    : DetectionResult(Source::CAMERA_SNAPSHOT, evalBoard)
{
}


/////////////////////////////////////////////////////
/////////////////////////////////////////////////////


DetectionResultLive::DetectionResultLive(const std::shared_ptr<EvaluatedBoard>& evalBoard)
    : DetectionResult(Source::CAMERA_LIVE, evalBoard)
{
}