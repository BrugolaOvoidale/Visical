#include "OverexposureCheck.hpp"


OverexposureCheck::OverexposureCheck(double threshold)
    : Plugin(
        std::string(ID),
        std::string(NAME),
        std::string(DESCRIPTION),
        threshold)
{
}

/////////////////////////////////////////////////////////////

std::shared_ptr<OverexposureCheck> OverexposureCheck::create(double threshold)
{
    struct Enabler : public OverexposureCheck
    {
        Enabler(double thr) : OverexposureCheck(thr) {}
    };

    return std::make_shared<Enabler>(threshold);
}

/////////////////////////////////////////////////////////////

std::shared_ptr<PluginResult> OverexposureCheck::executeImpl(const std::shared_ptr<Board>& board) const
{
    if (!board->isDetected())
    {
        return executionFailed("Cannot evaluate a non-detected board");
    }

    const CvRegion boardRegion = board->boardContour().toRegion(true);

    // 1. Compute board area
    double boardArea = boardRegion.areaCenter();

    if (boardArea <= 0.0)
    {
        return executionFailed("Board region invalid");
    }

    // 2. Reduce domain
    CvImage boardImage = board->image().reduceDomain(boardRegion).toGray();

    // 3. Threshold to find overexposed pixels
    CvRegion overexpRegion = boardImage.threshold(minThreshold, maxThreshold);

    // 4. Compute overexposed area
    double areaOverexp = overexpRegion.areaCenter();

    // 5. Compute ratio and quality
    double currRatio = areaOverexp / boardArea;
    double quality = 0.0;
    if (currRatio < maxAreaRatio)
        quality = std::min(1.0 - currRatio / maxAreaRatio, 1.0) * 100.0;


    return executionResult(
        quality,
        std::make_shared<OverexposureCheckDebug>(
            boardRegion,
            overexpRegion,
            boardArea,
            areaOverexp
        )
    );
}

void OverexposureCheck::validateParameters() const
{
    if (minThreshold < 0 || minThreshold > 254)
        throw std::invalid_argument("maxThreshold must be in the range (0, 254]");

    if (minThreshold >= maxThreshold)
        throw std::invalid_argument("minThreshold must be less than maxThreshold");

    if (maxAreaRatio <= 0.0 || maxAreaRatio > 1.0)
		throw std::invalid_argument("maxAreaRatio must be in the range (0, 1]");
}

std::string OverexposureCheck::getMessageForSeverity(EvaluationSeverity severity) const
{
    switch (severity)
    {
    case EvaluationSeverity::FAILED:
        return "Board check failed";
    case EvaluationSeverity::CRITICAL:
        return "Board is overexposed";
    case EvaluationSeverity::WARNING:
        return "Board is slightly overexposed";
    case EvaluationSeverity::OK:
    default:
        return "";
    }
}