#include "FocusCheck.hpp"
#include <cv/CvImage.hpp>
#include <cv/CvRegion.hpp>
#include <numeric>


static double trimmedMean(
    std::vector<double> vals,
    double trimFraction)
{
    if (vals.empty()) return 0.0;

    std::sort(vals.begin(), vals.end());

    const size_t trimCount = std::max<size_t>(1, static_cast<size_t>(vals.size() * trimFraction));

    const size_t lo = (vals.size() > 2 * trimCount) ? trimCount : 0;

    const size_t hi = (vals.size() > 2 * trimCount) ? vals.size() - trimCount : vals.size();

    return std::accumulate(vals.begin() + lo, vals.begin() + hi, 0.0) /
        static_cast<double>(hi - lo);
}

/////////////////////////////////////////////////////////////

FocusCheck::FocusCheck(double threshold)
    : Plugin(
        std::string(ID),
        std::string(NAME),
        std::string(DESCRIPTION),
        { DEPENDENCIES.begin(), DEPENDENCIES.end() },
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

std::shared_ptr<PluginResult> FocusCheck::executeImpl(
    const std::shared_ptr<Board>& board,
    const std::unordered_map<std::string, std::shared_ptr<PluginResult>>& producersResults) const
{
    if (!board->isDetected())
        return executionFailed("Cannot evaluate a non-detected board");

    try
    {
        CvImage image = board->image().toGray();

        const std::vector<CvContour>& marksContours = board->marksContours();
        if (marksContours.size() < minMarksCount)
            return executionFailed("Too few calibration marks detected");

        // Get board contrast
        const double contrast = std::static_pointer_cast<ConstrastCheckDebug>(
            producersResults.at(std::string(ContrastCheck::ID))->debugResult())
            ->contrast();

        if (contrast == 0.0)
            return executionFailed("Contrast too low to evaluate focus");

        // Gauss derivative
        CvImage edgeImage = image.derivateGauss(sigma);

        // Mean gradient loop
        std::vector<double> meanGradient;
        meanGradient.reserve(marksContours.size());

        for (const auto& singleMark : marksContours)
        {
            CvRegion singleMarkEdge = CvRegion::fromContours({ singleMark }, false);
            CvImage croppedImage = edgeImage.crop(singleMarkEdge.boundingBox());

            std::vector<double> singleMean;
            croppedImage.meanStdDev(singleMarkEdge, &singleMean, nullptr);

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