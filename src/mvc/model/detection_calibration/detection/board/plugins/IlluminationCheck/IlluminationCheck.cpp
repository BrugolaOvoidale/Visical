#include "IlluminationCheck.hpp"


IlluminationCheck::IlluminationCheck(double threshold)
    : Plugin(
        std::string(ID),
        std::string(NAME),
        std::string(DESCRIPTION),
        threshold)
{
}

/////////////////////////////////////////////////////////////

std::shared_ptr<IlluminationCheck> IlluminationCheck::create(double threshold)
{
    struct Enabler : public IlluminationCheck
    {
        Enabler(double thr) : IlluminationCheck(thr) {}
    };

    return std::make_shared<Enabler>(threshold);
}

/////////////////////////////////////////////////////////////

std::shared_ptr<PluginResult> IlluminationCheck::executeImpl(const std::shared_ptr<Board>& board) const
{
    if (!board->isDetected())
        return executionFailed("Cannot evaluate a non-detected board");

    try
    {
        // Setup
        CvImage image = board->image().toGray();
        const std::vector<CvContour>& marksContours = board->marksContours();

        std::vector<std::vector<CvContour>> dilatedMarks;
        dilatedMarks.reserve(marksContours.size());
        std::vector<double> maxVals;
        maxVals.reserve(marksContours.size());

        for (const auto& contour : marksContours)
        {
            CvRegion localRegion = CvRegion::fromContours({ contour }, false).dilationCircle(markDilationRadius);

            dilatedMarks.push_back(localRegion.toContours(CvRegion::ContourMode::BORDER_HOLES));

            CvImage croppedImage = image.crop(localRegion.boundingBox());

            std::vector<double> singleMaxVal;
            croppedImage.minMaxGray(localRegion, 3, nullptr, &singleMaxVal, nullptr);

            if (!singleMaxVal.empty())
                maxVals.push_back(singleMaxVal.front());
        }

        if (maxVals.empty())
            return executionFailed("No marks to evaluate illumination");

        // Statistics
        double mean = 0.0;
        for (double v : maxVals)
            mean += v;
        mean /= maxVals.size();

        std::vector<IlluminationCheckDebug::Mark> markScores;
        markScores.reserve(maxVals.size());
        double var = 0.0;

        for (double val : maxVals)
        {
            double deviation = std::abs(val - mean);
            double homogeneityScore = scoreOffsetFactor - (deviation / intensityDeviationScale);
            double score = std::clamp(homogeneityScore, 0.0, 1.0) * 100.0;
            markScores.emplace_back(score, val, deviation, evaluateScore(score));
            var += deviation * deviation;
        }

        var /= maxVals.size();
        const double stdDev = std::sqrt(var);
        double homogeneityScore = scoreOffsetFactor - (stdDev / intensityDeviationScale);
        double quality = std::clamp(homogeneityScore, 0.0, 1.0) * 100.0;

        return executionResult(
            quality,
            std::make_shared<IlluminationCheckDebug>(
                std::move(dilatedMarks),
                mean,
                stdDev,
                std::move(markScores)
            )
        );
    }
    catch (const std::exception& e)
    {
        return executionFailed("Error checking illumination: " + std::string(e.what()));
    }
}

void IlluminationCheck::validateParameters() const
{
    if (markDilationRadius <= 0.0)
    {
        throw std::invalid_argument("markDilationRadius must be greater than 0");
	}

    if (intensityDeviationScale <= 0.0)
    {
        throw std::invalid_argument("Standard deviation normalization factor must be positive");
	}

    if (scoreOffsetFactor < 0.0)
    {
        throw std::invalid_argument("Score offset factor must be greater than 0");
	}
}

std::string IlluminationCheck::getMessageForSeverity(EvaluationSeverity severity) const
{
    switch (severity)
    {
    case EvaluationSeverity::FAILED:
        return "Illumination check failed";
    case EvaluationSeverity::CRITICAL:
        return "Illumination is inhomogeneous";
    case EvaluationSeverity::WARNING:
        return "Illumination is slightly inhomogeneous";
    case EvaluationSeverity::OK:
    default:
        return "";
    }
}
