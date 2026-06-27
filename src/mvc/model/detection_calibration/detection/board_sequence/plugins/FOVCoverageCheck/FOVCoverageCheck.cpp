#include "FOVCoverageCheck.hpp"
#include <numeric>
#include <cv/CvRegion.hpp>


FOVCoverageCheck::FOVCoverageCheck(double threshold)
    : Plugin(
        std::string(ID),
        std::string(NAME),
        std::string(DESCRIPTION),
        {},
        threshold)
{
}

/////////////////////////////////////////////////////////////

std::shared_ptr<FOVCoverageCheck> FOVCoverageCheck::create(double threshold)
{
    struct Enabler : public FOVCoverageCheck
    {
        Enabler(double thr) : FOVCoverageCheck(thr) {}
    };

    return std::make_shared<Enabler>(threshold);
}

/////////////////////////////////////////////////////////////

std::shared_ptr<PluginResult> FOVCoverageCheck::executeImpl(
    const std::vector<std::shared_ptr<Board>>& boards,
    const std::unordered_map<std::string, std::shared_ptr<PluginResult>>& producersResults) const
{
    try
    {
        if (boards.empty())
        {
            return executionResult(
                0.0,
                std::make_shared<FOVCoverageCheckDebug>(BLANK_HEATMAP)
            );
        }

        std::vector<std::vector<cv::Point2f>> imagePoints;
        std::vector<double> marksDiameters;
        imagePoints.reserve(boards.size());

        for (const auto& board : boards)
        {
            if (!board->isDetected())
                continue;

            imagePoints.push_back(board->imagePoints());
            
            for (const auto& contour : board->marksContours())
            {
                double diameter;
                if (contour.diameter(&diameter))
                {
                    marksDiameters.push_back(diameter);
                }
            }
        }

        // Validate inputs
        if (imagePoints.empty())
            return executionFailed("Observation points invalid");

        const double avgMarksDiameter = marksDiameters.empty() ? 0.0 :
            std::accumulate(marksDiameters.begin(), marksDiameters.end(), 0.0) /
            marksDiameters.size();

        const int ImageWidth = boards.front()->image().width();
        const int ImageHeight = boards.front()->image().height();

        if (ImageWidth <= 0 || ImageHeight <= 0)
            return executionFailed("Invalid image size for heatmap");


        CvRegion pointsRegion = CvRegion::fromPoints(imagePoints);

        // 3) Distance transform (float image)
        CvImage distanceImage = CvImage::distanceTransform(pointsRegion, ImageWidth, ImageHeight);

        // 4) Create border mask to exclude unreliable edges
        double Border = std::max(ImageWidth, ImageHeight) * borderExclusionRatio;
        int r1 = static_cast<int>(std::round(Border));
        int c1 = static_cast<int>(std::round(Border));
        int r2 = ImageHeight - 1 - r1;
        int c2 = ImageWidth - 1 - c1;
        if (r2 <= r1 || c2 <= c1)
            return executionFailed("Image too small for border exclusion");

        CvRegion borderMask = CvRegion::fromRectangle(r1, c1, r2, c2);

        // 5) Get maximum distance value within border mask
        std::vector<double> maxVals;
        // distanceImage is float; use cvImage::minMaxGray with mask
        distanceImage.minMaxGray(borderMask, 3, nullptr, &maxVals, nullptr);

        if (maxVals.empty())
            return executionFailed("No max val found");

		
        double maxVal = *std::max_element(maxVals.begin(), maxVals.end());

        // 6) Compute image diagonal
        double imageDiagonal = std::hypot(static_cast<double>(ImageHeight - 1), static_cast<double>(ImageWidth - 1));

        // 7) Compute Ratio and score exactly
        double Ratio = 0.0;
        if (imageDiagonal > 0.0)
            Ratio = (maxVal / imageDiagonal) * coverageNormalizationFactor;

        double Tmp1 = 1.0 - Ratio;
        double Tmp2 = (Tmp1 - minCoverageThreshold) / (maxCoverageThreshold - minCoverageThreshold);
        double quality = std::clamp(Tmp2, 0.0, 1.0) * 100.0;

        // 8) Optional visualization (dilate region by diam*0.1 then distance transform)
        //CvRegion regionDilation = pointsRegion.dilationCircle(avgMarksDiameter);   //TODO: is awfully SLOW with circle ~diam !!!

        // 10) Return result using same thresholds
        return executionResult(
            quality,
            std::make_shared<FOVCoverageCheckDebug>(
                distanceImage.scaleImageMax().bitwiseNot()
            )
        );
    }
    catch (const std::exception& ex)
    {
        return executionFailed(std::string("Error in CheckFOVCoverage: ") + ex.what());
    }
}

void FOVCoverageCheck::validateParameters() const
{
    if (borderExclusionRatio <= 0.0)
        throw std::runtime_error("borderExclusionRatio must be greater than 0");

    if (coverageNormalizationFactor <= 0.0)
        throw std::runtime_error("coverageNormalizationFactor must be greater than 0");

    if (minCoverageThreshold <= 0.0)
        throw std::runtime_error("minCoverageThreshold must be greater than 0");

    if (maxCoverageThreshold <= 0.0)
        throw std::runtime_error("maxCoverageThreshold must be greater than 0");

    if (minCoverageThreshold >= maxCoverageThreshold)
        throw std::runtime_error("minCoverageThreshold must be lower than maxCoverageThreshold");
}

std::string FOVCoverageCheck::getMessageForSeverity(EvaluationSeverity severity) const
{
    switch (severity)
    {
    case EvaluationSeverity::FAILED:
        return "FOV check failed";
    case EvaluationSeverity::CRITICAL:
        return "Field of view is not covered by board images";
    case EvaluationSeverity::WARNING:
        return "Field of view is not enough covered by board images";
    case EvaluationSeverity::OK:
    default:
        return "";
    }
}
