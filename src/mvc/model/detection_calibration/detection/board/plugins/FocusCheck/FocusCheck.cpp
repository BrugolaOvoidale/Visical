#include "FocusCheck.hpp"
#include <cv/CvImage.hpp>
#include <cv/CvRegion.hpp>


FocusCheck::FocusCheck(double threshold)
    : Plugin(
        std::string(ID),
        std::string(NAME),
        std::string(DESCRIPTION),
        threshold)
{
}

/////////////////////////////////////////////////////////////

std::shared_ptr<FocusCheck> FocusCheck::create(double threshold)
{
    struct Enabler : public FocusCheck
    {
        Enabler(double thr) : FocusCheck(thr) {}
    };

    return std::make_shared<Enabler>(threshold);
}

/////////////////////////////////////////////////////////////

std::shared_ptr<PluginResult> FocusCheck::executeImpl(const std::shared_ptr<Board>& board) const
{
    if (!board->isDetected())
        return executionFailed("Cannot evaluate a non-detected board");

    try
    {
        const CvImage& image = board->image();

        const std::vector<CvContour>& marksContours = board->marksContours();
        if (marksContours.size() < minMarksCount)
            return executionFailed("Too few calibration marks detected");

        const CvRegion marksRegion = CvRegion::fromContours(marksContours, false);
        std::vector<CvRegion> singleMarksRegions;
        singleMarksRegions.reserve(marksContours.size());

        // Find Contrast
        double contrast{ 0.0 };
        std::vector<double> range;
        {
            range.reserve(marksContours.size());

            for (const auto& contour : marksContours)
            {
                CvRegion singleMark = CvRegion::fromContours({ contour }, false);
                CvRegion dilatedSingle = singleMark.dilationCircle(markDilationRadius);

                CvImage croppedImage = image.crop(dilatedSingle.boundingBox());         // small subimage

                std::vector<double> singleRange;
                croppedImage.toGray().minMaxGray(dilatedSingle, 3, nullptr, nullptr, &singleRange);

                singleMarksRegions.push_back(std::move(singleMark));

                if (!singleRange.empty())
                    range.push_back(singleRange.front());
            }

            if (range.empty())
                return executionFailed("Contrast too low to evaluate focus");

            double tot = 0.0;
            for (double r : range) tot += r;
            contrast = tot / range.size();

            if (contrast == 0.0)
                return executionFailed("Contrast too low to evaluate focus");
        }

        // Gauss derivative
        CvImage edgeImage = image.derivateGauss(sigma);

        // Mean gradient loop
        std::vector<double> meanGradient;
        meanGradient.reserve(marksContours.size());

        for (const auto& singleMark : singleMarksRegions)
        {
            CvImage croppedImage = edgeImage.crop(singleMark.boundingBox());

            std::vector<double> singleMean;
            croppedImage.meanStdDev(singleMark, &singleMean, nullptr);

            if (!singleMean.empty())
                meanGradient.push_back(singleMean.front());
        }

        if (meanGradient.empty())
            return executionFailed("No marks to evaluate focus");

        // Scoring
        std::vector<double> scores;
        scores.reserve(meanGradient.size());
        std::vector<FocusCheckDebug::Mark> markScores;
        markScores.reserve(meanGradient.size());

        for (const double meanGrad : meanGradient)
        {
            double raw = meanGrad / contrast;
            double scaled = raw * gradientScalingFactor;
            double score = std::clamp(scaled - minScoreOffset, 0.0, 1.0) * 100.0;

            markScores.emplace_back(score, meanGrad, raw, evaluateScore(score));
            scores.push_back(score);
        }

        std::sort(scores.begin(), scores.end());
        size_t index = std::clamp(
            static_cast<size_t>(std::round(
                static_cast<double>(scores.size()) / gradientPercentileDivisor
            )),
            size_t{ 0 },
            scores.size() - 1
        );
        double boardScore = scores[index];

        return executionResult(
            boardScore,
            std::make_shared<FocusCheckDebug>(
                std::move(edgeImage),
                contrast,
                std::move(markScores)
            )
        );
    }
    catch (const std::exception& e)
    {
        return executionFailed("Error checking focus: " + std::string(e.what()));
    }
}

void FocusCheck::validateParameters() const
{
    if (minMarksCount == 0)
		throw std::runtime_error("minMarksCount must be greater than 0");

    if (sigma <= 0.0)
		throw std::runtime_error("sigma must be greater than 0");

    if (gradientScalingFactor <= 0.0)
		throw std::runtime_error("gradientScalingFactor must be greater than 0");

	if (gradientPercentileDivisor <= 0.0)
        throw std::runtime_error("gradientPercentileDivisor must be greater than 0");

    if (minScoreOffset < 0.0)
		throw std::runtime_error("minScoreOffset must be non-negative");
}

std::string FocusCheck::getMessageForSeverity(EvaluationSeverity severity) const
{
    switch (severity)
    {
    case EvaluationSeverity::FAILED:
        return "FocusCheck check failed";
    case EvaluationSeverity::CRITICAL:
        return "Mark on the board are out of focus. Image appears blurry";
    case EvaluationSeverity::WARNING:
        return "Mark may be slightly out of focus. Consider improving focus";
    case EvaluationSeverity::OK:
    default:
        return "";
    }
}