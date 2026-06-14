#include "ContrastCheck.hpp"
#include <cv/CvRegion.hpp>


ContrastCheck::ContrastCheck(double threshold)
    : Plugin(
        std::string(ID),
        std::string(NAME),
        std::string(DESCRIPTION),
        threshold)
{
}

/////////////////////////////////////////////////////////////

std::shared_ptr<ContrastCheck> ContrastCheck::create(double threshold)
{
    struct Enabler : public ContrastCheck
    {
        Enabler(double thr) : ContrastCheck(thr) {}
	};

    return std::make_shared<Enabler>(threshold);
}

/////////////////////////////////////////////////////////////

std::shared_ptr<PluginResult> ContrastCheck::executeImpl(const std::shared_ptr<Board>& board) const
{
    if (!board->isDetected())
    {
        return executionFailed("Cannot evaluate a non-detected board");
    }

    try
    {
        CvImage image = board->image().toGray();
        const std::vector<CvContour>& marksContours = board->marksContours();

        // Dilate the marks region
        std::vector<std::vector<CvContour>> dilatedMarks;
        dilatedMarks.reserve(marksContours.size());

        // Compute range gray value inside analysis region
        std::vector<double> range;
        range.reserve(marksContours.size());

        for (const auto& contour : marksContours)
        {
            CvRegion dilatedSingle = CvRegion::fromContours({ contour }, false).dilationCircle(markDilationRadius);

            dilatedMarks.push_back(dilatedSingle.toContours(CvRegion::ContourMode::BORDER_HOLES));

            CvImage croppedImage = image.crop(dilatedSingle.boundingBox());    // small subimage

            std::vector<double> singleRange;
            croppedImage.minMaxGray(dilatedSingle, 3, nullptr, nullptr, &singleRange);

            if (!singleRange.empty())
                range.push_back(singleRange.front());
        }

        if (range.empty())
            return executionFailed("No marks contrast computed");

        double totScore = 0.0;
        std::vector<ConstrastCheckDebug::Mark> marksScores;
        marksScores.reserve(range.size());
        for (const auto& r : range)
        {
            double contrastScore = (r - minDetectableContrast) / (sufficientContrastThr - minDetectableContrast);
            double score = std::min(1.0, contrastScore) * 100.0;

            marksScores.emplace_back(
                score,
                r,
                evaluateScore(score)
            );

            totScore += score;
        }

        double boardScore = totScore / range.size();

        return executionResult(
            boardScore,
            std::make_shared<ConstrastCheckDebug>(
                std::move(dilatedMarks),
                std::move(marksScores)
            )
        );
    }
    catch (const std::exception& e)
    {
        return executionFailed(std::string("Error checking illumination: ") + e.what());
    }
}

void ContrastCheck::validateParameters() const
{
    if (markDilationRadius <= 0)
    {
        throw std::invalid_argument("markDilationRadius must be greater than 0");
	}

    if (minDetectableContrast < 0 || minDetectableContrast > 254)
    {
        throw std::invalid_argument("minDetectableContrast must be in the range (0, 254]");
    }

    if (minDetectableContrast >= sufficientContrastThr)
    {
        throw std::invalid_argument("minDetectableContrast must be lesser than sufficientContrastThr");
	}
}

std::string ContrastCheck::getMessageForSeverity(EvaluationSeverity severity) const
{
    switch (severity)
    {
    case EvaluationSeverity::FAILED:
        return "Contrast check failed";
    case EvaluationSeverity::CRITICAL:
        return "Contrast is low";
    case EvaluationSeverity::WARNING:
        return "Contrast is slightly low";
    case EvaluationSeverity::OK:
    default:
        return "";
    }
}