#include "DetectionResultView.hpp"
#include <mvc/model/detection_calibration/detection/board_sequence/EvaluatedBoardSequence.hpp>


DetectionResultView::DetectionResultView(
    const DetectionResultMap::Entry& boardRes,
    const wxImage& boardConvImage)
    : boardRes_(boardRes),
    boardConvImage_(boardConvImage)
{
}

////////////////////////////////////////////////////////

const DetectionResultMap::Entry& DetectionResultView::boardResultEntry() const
{
    return boardRes_;
}

const wxImage& DetectionResultView::wxBoardImage() const
{
    return boardConvImage_;
}
