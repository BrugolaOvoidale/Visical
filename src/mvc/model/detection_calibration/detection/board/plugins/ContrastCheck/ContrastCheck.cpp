#include "ContrastCheck.hpp"
#include <numeric>
#include <cv/CvRegion.hpp>


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

ContrastCheck::ContrastCheck(double threshold)
    : Plugin(
        std::string(ID),
        std::string(NAME),
        std::string(DESCRIPTION),
        {},
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

std::shared_ptr<PluginResult> ContrastCheck::executeImpl(
    const std::shared_ptr<Board>& board,
    const std::unordered_map<std::string, std::shared_ptr<PluginResult>>& producersResults) const
{
    if (!board->isDetected())
    {
        return executionFailed("Cannot evaluate a non-detected board");
    }

    try
    {
        const BoardPattern boardPattern = board->patternType();
        CvImage image = board->image().toGray();
        const std::vector<CvContour>& marksContours = board->marksContours();

        std::vector<std::vector<CvContour>> debugContours;
        debugContours.reserve(marksContours.size());

        std::vector<double> range;
        range.reserve(marksContours.size());

        // Tessellated grids (chessboard / charuco / apriltag).
        // Contrast is defined cross-cell: how far does a dark cell's darkest core fall
        // below the board's global white level, and vice versa?
        // Both metrics land on a [0, 255] pixel-intensity scale, matching the
        // circle-grid path below, so minDetectableContrast / sufficientContrastThr
        // apply uniformly to both paths.
        if (boardPattern == BoardPattern::CHESSBOARD || boardPattern == BoardPattern::CHARUCO || boardPattern == BoardPattern::APRIL_TAG)
        {
            // Structure to pass metrics from the first classification pass to the scoring pass
            struct CellMetrics {
                double minVal;
                double maxVal;
                double meanVal;
                CvRegion region;
            };
            std::vector<CellMetrics> cellsMetrics;
            cellsMetrics.reserve(marksContours.size());

            // Pass 1: erode each cell's core to avoid edge transitions, then sample
            // min, max, and mean intensity.
            // We erode with a rectangle (not a circle) because chessboard/charuco cells
            // are themselves rectangular; a circle erode would clip corners needlessly.
            // The circlegrid path below dilates with a circle because isolated blobs are
            // roughly circular, so the two operations are asymmetric by design.
            for (const auto& contour : marksContours)
            {
                // Erode slightly to sample the inner core, avoiding edge transitions
                CvRegion erodedSingle = CvRegion::fromContours({ contour }, true).erosionRectangle(kernelSize, kernelSize);
                CvImage croppedImage = image.crop(erodedSingle.boundingBox());

                std::vector<double> minVal, maxVal;

                // Harvest the true spatial mean alongside min/max so that
                // dark/light classification is robust against isolated bright or dark
                // outlier pixels.
                croppedImage.minMaxGray(erodedSingle, 3, &minVal, &maxVal, nullptr);

                std::vector<double> meanVal;
                croppedImage.meanStdDev(erodedSingle, &meanVal, nullptr);

                cellsMetrics.push_back({
                    minVal.front(),
                    maxVal.front(),
                    meanVal.front(),
                    erodedSingle
                    });
            }

            // Median of all cell means as the dark/light split point..
            // The median is insensitive to outliers and still correctly bisects a
            // bimodal chessboard histogram.
            std::vector<double> sortedMeans;
            sortedMeans.reserve(cellsMetrics.size());
            for (const auto& c : cellsMetrics)
                sortedMeans.push_back(c.meanVal);

            const size_t mid = sortedMeans.size() / 2;
            std::nth_element(sortedMeans.begin(), sortedMeans.begin() + mid, sortedMeans.end());
            const double dynamicThreshold = sortedMeans[mid];

            // Trim extreme cells before building the global baselines.
            // Collecting raw minVal / maxVal and averaging them means a single
            // blown-out white cell or pitch-black smear would shift every contrast
            // score on the board. We discard the top/bottom 10 % of each bin
            // (clamped to at least 1 element so small boards are handled) before
            // averaging.
            std::vector<double> darkMinVals, lightMaxVals;

            for (const auto& cell : cellsMetrics)
            {
                if (cell.meanVal < dynamicThreshold)
                    darkMinVals.push_back(cell.minVal);
                else
                    lightMaxVals.push_back(cell.maxVal);
            }

            const double globalAvgDark = darkMinVals.empty() ? 0.0 : trimmedMean(darkMinVals, 0.1);
            const double globalAvgLight = lightMaxVals.empty() ? 255.0 : trimmedMean(lightMaxVals, 0.1);

            // Pass 2: per-cell cross-contrast against the trimmed global baselines.
            for (const auto& cell : cellsMetrics)
            {
                double cellContrast = 0.0;
                if (cell.meanVal < dynamicThreshold)
                {
                    // Dark cell: how far below the reliable white baseline is its darkest core?
                    cellContrast = globalAvgLight - cell.minVal;
                }
                else
                {
                    // Light cell (white square or ArUco background): how far above the
                    // reliable dark baseline is its brightest region?
                    cellContrast = cell.maxVal - globalAvgDark;
                }

                range.push_back(cellContrast);
                debugContours.push_back(cell.region.toContours(CvRegion::ContourMode::BORDER_HOLES));
            }
        }
        // Isolated patterns (circle grids).
        // Contrast is intra-mark: the min-to-max pixel range sampled across the
        // dilated blob region (circle + its immediate background ring).
        // The dilation is circular to match the blob geometry; the tessellated path
        // above uses rectangular erosion to match cell geometry..
        // The resulting range value is on the same [0, 255] intensity scale as the
        // cross-contrast values above, so the shared scoring formula below is valid.
        else if (boardPattern == BoardPattern::SYMMETRIC_CIRCLES || boardPattern == BoardPattern::ASYMMETRIC_CIRCLES)
        {
            for (const auto& contour : marksContours)
            {
                CvRegion dilatedSingle = CvRegion::fromContours({ contour }, false).dilationCircle(kernelSize);
                debugContours.push_back(dilatedSingle.toContours(CvRegion::ContourMode::BORDER_HOLES));

                CvImage croppedImage = image.crop(dilatedSingle.boundingBox());
                std::vector<double> singleRange;
                croppedImage.minMaxGray(dilatedSingle, 3, nullptr, nullptr, &singleRange);

                if (!singleRange.empty())
                    range.push_back(singleRange.front());
            }
        }

        if (range.empty())
            return executionFailed("No marks contrast computed");

        const double totContrast = std::accumulate(range.begin(), range.end(), 0.0);
        const double contrast = totContrast / range.size();

        double totScore = 0.0;
        std::vector<ConstrastCheckDebug::Mark> marksScores;
        marksScores.reserve(range.size());

        for (const auto& r : range)
        {
            double contrastScore = (r - minDetectableContrast) / (sufficientContrastThr - minDetectableContrast);

            double score = std::clamp(contrastScore, 0.0, 1.0) * 100.0;

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
                std::move(debugContours),
                contrast,
                std::move(marksScores)
            )
        );
    }
    catch (const std::exception& e)
    {
        return executionFailed(std::string("Error checking contrast: ") + e.what());
    }
}

void ContrastCheck::validateParameters() const
{
    if (kernelSize <= 0)
    {
        throw std::invalid_argument("kernelSize must be greater than 0");
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