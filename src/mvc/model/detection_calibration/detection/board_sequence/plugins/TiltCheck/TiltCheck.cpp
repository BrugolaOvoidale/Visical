#include "TiltCheck.hpp"
#include <numbers>
#include <opencv2/calib3d.hpp>


TiltCheck::TiltCheck(double threshold)
    : Plugin(
        std::string(ID),
        std::string(NAME),
        std::string(DESCRIPTION),
        threshold)
{
}

/////////////////////////////////////////////////////////////

std::shared_ptr<TiltCheck> TiltCheck::create(double threshold)
{
    struct Enabler : public TiltCheck
    {
        Enabler(double thr) : TiltCheck(thr) {}
    };

    return std::make_shared<Enabler>(threshold);
}

/////////////////////////////////////////////////////////////

std::shared_ptr<PluginResult> TiltCheck::executeImpl(const std::vector<std::shared_ptr<Board>>& boards) const
{
    if (boards.empty())
    {
        return executionNotApplicable("No boards provided");
    }

    // std::vector<std::optional<TiltCheckDebug::BoardPose>> boardEntries;
    // boardEntries.reserve(boards.size());

    std::vector<double> slantScores, panScores;

    for (const auto& board : boards)
    {
        if (!board->hasPose())
        {
            // boardEntries.push_back(std::nullopt);
            continue;
        }

        const cv::Mat& rvec = board->pose().rvec();

        // Convert rotation vector to rotation matrix
        cv::Mat R;
        cv::Rodrigues(rvec, R);

        // Extract Euler angles
        double pitch, yaw, roll;
        extractEulerAngles(R, pitch, yaw, roll);

        // Convert to degrees
        double pitchDeg = pitch * 180.0 / std::numbers::pi;
        double yawDeg = yaw * 180.0 / std::numbers::pi;
        double rollDeg = roll * 180.0 / std::numbers::pi;

        // Normalize angles to [-180, 180] range
        double slant = std::fmod(pitchDeg + 180.0, 360.0) - 180.0;
        double pan = std::fmod(yawDeg + 180.0, 360.0) - 180.0;

        double normSlant = std::min(std::abs(slant), maxAngleCap) * angleScalingTarget / maxAngleCap;
        double normPan = std::min(std::abs(pan), maxAngleCap) * angleScalingTarget / maxAngleCap;

        double slantScore = computeSingleScore(normSlant);
        double panScore = computeSingleScore(normPan);

        slantScores.push_back(slantScore);
        panScores.push_back(panScore);

        // boardEntries.push_back(
        //     TiltCheckDebug::BoardPose{ slantScore, panScore,
        //     pitchDeg, yawDeg, rollDeg,
        //     normSlant, normPan }
        // );
    }

    if (slantScores.empty())
        return executionFailed("No valid poses available for tilt check");

    std::sort(slantScores.begin(), slantScores.end(), std::greater<>());
    std::sort(panScores.begin(), panScores.end(), std::greater<>());

    int numUsed = std::min(static_cast<int>(minImages), static_cast<int>(slantScores.size()));

    double SlantScore = 0.0, PanScore = 0.0;
    for (int i = 0; i < numUsed; ++i)
    {
        SlantScore += slantScores[i];
        PanScore += panScores[i];
    }

    SlantScore /= minImages;
    PanScore /= minImages;

    double quality = std::clamp(((SlantScore * slantWeight) + (PanScore * panWeight)), 0.0, 1.0) * 100.0;

    return executionResult(
        quality/*,
        std::make_shared<TiltCheckDebug>(
            std::move(boardEntries),
            slantScores,
            panScores,
            numUsed,
            SlantScore,
            PanScore
        )*/
    );
}

void TiltCheck::validateParameters() const
{
    if (minImages == 0)
		throw std::invalid_argument("minImages must be greater than 0");
}

void TiltCheck::extractEulerAngles(
    const cv::Mat& R,
    double& slant,
    double& pan,
    double& roll
) const
{
    // Extract Euler angles from rotation matrix (ZYX convention)
    // Rz (roll) -> Ry (pan) -> Rx (slant/tilt)

    double sy = std::sqrt(
        R.at<double>(0, 0) * R.at<double>(0, 0) +
        R.at<double>(1, 0) * R.at<double>(1, 0)
    );

    // Testing whether the rotation matrix is close to a singular configuration
    bool singular = sy < 1e-6;

    if (!singular)
    {
        slant = std::atan2(R.at<double>(2, 1), R.at<double>(2, 2));  // Rx (tilt)
        pan = std::atan2(-R.at<double>(2, 0), sy);                   // Ry (pan)
        roll = std::atan2(R.at<double>(1, 0), R.at<double>(0, 0));   // Rz (roll)
    }
    else
    {
        slant = std::atan2(-R.at<double>(1, 2), R.at<double>(1, 1));
        pan = std::atan2(-R.at<double>(2, 0), sy);
        roll = 0;
    }
}

double TiltCheck::computeSingleScore(double normalizedAngle) const
{
    double s = 1.0 - std::abs(std::pow(
        std::cos(normalizedAngle * 2 * std::numbers::pi / 180.0),
        scoreExponent));
    return std::min(s * scoreAdjustmentMultiplier, 1.0);
}

std::string TiltCheck::getMessageForSeverity(EvaluationSeverity severity) const
{
    switch (severity)
    {
    case EvaluationSeverity::FAILED:
        return "Tilt check failed";
    case EvaluationSeverity::CRITICAL:
        return "Tilt angles are not covered by sequence";
    case EvaluationSeverity::WARNING:
        return "Tilt angles are not enough covered by sequence";
    case EvaluationSeverity::OK:
    default:
        return "";
    }
}