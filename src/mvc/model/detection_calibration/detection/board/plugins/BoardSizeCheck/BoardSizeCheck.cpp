#include "BoardSizeCheck.hpp"
#include <opencv2/calib3d.hpp>


BoardSizeCheck::BoardSizeCheck(double threshold)
    : Plugin(
        std::string(ID),
        std::string(NAME),
        std::string(DESCRIPTION),
        threshold)
{
}

/////////////////////////////////////////////////////////////

std::shared_ptr<BoardSizeCheck> BoardSizeCheck::create(double threshold)
{

    struct Enabler : BoardSizeCheck
    {
        Enabler(double threshold)
            : BoardSizeCheck(threshold)
        {
        }
	};

    return std::make_shared<Enabler>(threshold);
}

/////////////////////////////////////////////////////////////

std::shared_ptr<PluginResult> BoardSizeCheck::executeImpl(const std::shared_ptr<Board>& board) const
{
    if (!board->isDetected())
        return executionFailed("Cannot evaluate a non-detected board");

    const int width = board->image().width();
    const int height = board->image().height();
    const double observedArea = board->boardContour().area();

    cv::Mat R;
    cv::Rodrigues(board->pose().rvec(), R);  // Rotation matrix

    // Board normal in board coordinates (assuming Z=0 plane) -> [0,0,1]
    cv::Mat normalBoard = (cv::Mat_<double>(3, 1) << 0, 0, 1);
    cv::Mat normalCam = R * normalBoard;   // Transform to camera coordinates

    // cos(theta) = absolute value of Z-component (normalized)
    double cosTheta = std::abs(normalCam.at<double>(2, 0));  // |N * Z_camera|
    if (cosTheta < 1e-6) cosTheta = 1e-6;  // avoid div by zero

    double effectiveArea = observedArea / cosTheta;  // estimate fronto-parallel area

    double coverageFraction = effectiveArea / (width * height);
    double normalizedQuality = (coverageFraction / expectedCoverage) * 100.0;
    double quality = std::clamp(normalizedQuality, 0.0, 100.0);

    return executionResult(quality);
}

void BoardSizeCheck::validateParameters() const
{
    if (expectedCoverage <= 0.0 || expectedCoverage > 1.0)
    {
        throw std::invalid_argument("Expected coverage must be in the range (0.0, 1.0]");
	}
}

std::string BoardSizeCheck::getMessageForSeverity(EvaluationSeverity severity) const
{
    switch (severity)
    {
    case EvaluationSeverity::FAILED:
        return "Board size check failed";
    case EvaluationSeverity::CRITICAL:
        return "Board in image is small";
    case EvaluationSeverity::WARNING:
        return "Board in image is slightly small";
    case EvaluationSeverity::OK:
    default:
        return "";
    }
}
