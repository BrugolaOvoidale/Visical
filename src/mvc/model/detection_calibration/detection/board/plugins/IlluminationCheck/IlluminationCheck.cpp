#include "IlluminationCheck.hpp"
#include <numeric>


IlluminationCheck::IlluminationCheck(double threshold)
    : Plugin(
        std::string(ID),
        std::string(NAME),
        std::string(DESCRIPTION),
        {},
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

std::shared_ptr<PluginResult> IlluminationCheck::executeImpl(
    const std::shared_ptr<Board>& board,
    const std::unordered_map<std::string, std::shared_ptr<PluginResult>>& producersResults) const
{
    if (!board->isDetected())
        return executionFailed("Cannot evaluate a non-detected board");

    try
    {
        // Setup
        const BoardPattern boardPattern = board->patternType();
        CvImage image = board->image().toGray();
        const std::vector<CvContour>& marksContours = board->marksContours();

        std::vector<std::vector<CvContour>> dilatedMarks;
        dilatedMarks.reserve(marksContours.size());

        std::vector<double> maxVals;
        maxVals.reserve(marksContours.size());

        // Tessalated grids (chessboard / charuco)
        if (boardPattern == BoardPattern::CHESSBOARD || boardPattern == BoardPattern::CHARUCO || boardPattern == BoardPattern::APRIL_TAG)
        {
            struct CellMetrics {
                CvRegion region;
                double maxVal;
                double meanVal;
                double centerX;
                double centerY;
                bool isLight;
            };
            std::vector<CellMetrics> cellsMetrics;
            cellsMetrics.reserve(marksContours.size());

            // Pass 1: Core sample all cells to gather metadata and coordinates
            for (const auto& contour : marksContours)
            {
                // filled = true to get a solid core patch, eroded to stay away from edges
                CvRegion erodedSingle = CvRegion::fromContours({ contour }, true).erosionRectangle(kernelSize, kernelSize);
                CvImage croppedImage = image.crop(erodedSingle.boundingBox());

                std::vector<double> minVal;
                std::vector<double> maxVal;
                std::vector<double> dummy;
                croppedImage.minMaxGray(erodedSingle, 3, &minVal, &maxVal, &dummy);

                const cv::Rect& rect = erodedSingle.boundingBox();
                double cx = rect.x + rect.width / 2.0;
                double cy = rect.y + rect.height / 2.0;

                cellsMetrics.push_back({ erodedSingle, maxVal.front(), (minVal.front() + maxVal.front()) / 2.0, cx, cy, false });
            }

            // Pass 2: Calculate automatic dynamic threshold for classification
            const double minMean = std::min_element(cellsMetrics.begin(), cellsMetrics.end(), [](const CellMetrics& a, const CellMetrics& b) { return a.meanVal < b.meanVal; })->meanVal;

            const double maxMean = std::max_element(cellsMetrics.begin(), cellsMetrics.end(), [](const CellMetrics& a, const CellMetrics& b) { return a.meanVal < b.meanVal; })->meanVal;

            const double dynamicThreshold = (minMean + maxMean) / 2.0;

            for (auto& cell : cellsMetrics)
            {
                if (cell.meanVal >= dynamicThreshold)
                    cell.isLight = true;
            }

            // Pass 3: Map precise local illumination to maintain 1:1 contour indexing
            for (const auto& cell : cellsMetrics)
            {
                double illuminationVal = 0.0;
                if (cell.isLight)
                {
                    // White cells map their own internal pristine white peak
                    illuminationVal = cell.maxVal;
                }
                else
                {
                    // Black cells borrow the illumination value of their closest white neighbor
                    double minDistanceSq = std::numeric_limits<double>::max();
                    double nearestWhiteMax = 255.0;

                    for (const auto& refCell : cellsMetrics)
                    {
                        if (!refCell.isLight) continue;

                        double dx = cell.centerX - refCell.centerX;
                        double dy = cell.centerY - refCell.centerY;
                        double distanceSq = dx * dx + dy * dy;

                        if (distanceSq < minDistanceSq)
                        {
                            minDistanceSq = distanceSq;
                            nearestWhiteMax = refCell.maxVal;
                        }
                    }
                    illuminationVal = nearestWhiteMax;
                }

                maxVals.push_back(illuminationVal);
                dilatedMarks.push_back(cell.region.toContours(CvRegion::ContourMode::BORDER_HOLES));
            }
        }
        // Isolated patterns (circle grids)
        else if (boardPattern == BoardPattern::SYMMETRIC_CIRCLES || boardPattern == BoardPattern::ASYMMETRIC_CIRCLES)
        {
            for (const auto& contour : marksContours)
            {
                // filled = false is perfect here; creates a hollow ring to span outwards
                CvRegion dilatedSingle = CvRegion::fromContours({ contour }, false).dilationCircle(kernelSize);
                dilatedMarks.push_back(dilatedSingle.toContours(CvRegion::ContourMode::BORDER_HOLES));

                CvImage croppedImage = image.crop(dilatedSingle.boundingBox());

                std::vector<double> singleMaxVal;
                croppedImage.minMaxGray(dilatedSingle, 3, nullptr, &singleMaxVal, nullptr);

                if (!singleMaxVal.empty())
                    maxVals.push_back(singleMaxVal.front());
            }
        }

        if (maxVals.empty())
            return executionFailed("No marks to evaluate illumination");

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
    if (kernelSize <= 0.0)
    {
        throw std::invalid_argument("kernelSize must be greater than 0");
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
