#include "RMSErrorCheck.hpp"
#include <algorithm>
#include <stdexcept>


RMSErrorCheck::RMSErrorCheck(double threshold)
    : Plugin(
        std::string(ID),
        std::string(NAME),
        std::string(DESCRIPTION),
        threshold)
{
}

/////////////////////////////////////////////////////////////

std::shared_ptr<RMSErrorCheck> RMSErrorCheck::create(double threshold)
{
    struct Enabler : public RMSErrorCheck
    {
        Enabler(double thr) : RMSErrorCheck(thr) {}
    };

    return std::make_shared<Enabler>(threshold);
}

/////////////////////////////////////////////////////////////

std::shared_ptr<PluginResult> RMSErrorCheck::executeImpl(const CalibratedBoard& calibratedBoard) const
{
    const double rmsError = calibratedBoard.RMSerror();

    // Quality inversely proportional to RMS error relative to threshold:
    // rmsError == 0          -> quality 100
    double normalizedQuality = (1.0 - (rmsError / maxRMSError)) * 100.0;
    double quality = std::clamp(normalizedQuality, 0.0, 100.0);

    return executionResult(quality);
}

void RMSErrorCheck::validateParameters() const
{
    if (maxRMSError <= 0.0)
        throw std::invalid_argument("maxRMSError must be positive");
}

std::string RMSErrorCheck::getMessageForSeverity(EvaluationSeverity severity) const
{
    switch (severity)
    {
    case EvaluationSeverity::FAILED:
        return "Reprojection error check failed";
    case EvaluationSeverity::CRITICAL:
        return "Board reprojection error is too high";
    case EvaluationSeverity::WARNING:
        return "Board reprojection error is slightly high";
    case EvaluationSeverity::OK:
    default:
        return "";
    }
}