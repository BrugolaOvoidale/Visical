#include "MaxErrorCheck.hpp"
#include <algorithm>
#include <stdexcept>


MaxErrorCheck::MaxErrorCheck(double threshold)
    : Plugin(
        std::string(ID),
        std::string(NAME),
        std::string(DESCRIPTION),
        threshold)
{
}

/////////////////////////////////////////////////////////////

std::shared_ptr<MaxErrorCheck> MaxErrorCheck::create(double threshold)
{
    struct Enabler : public MaxErrorCheck
    {
        Enabler(double thr) : MaxErrorCheck(thr) {}
    };

    return std::make_shared<Enabler>(threshold);
}

/////////////////////////////////////////////////////////////

std::shared_ptr<PluginResult> MaxErrorCheck::executeImpl(const CalibratedBoard& calibratedBoard) const
{
    const double maxError = calibratedBoard.maxError();

    double normalizedQuality = (1.0 - (maxError / maxMaxError)) * 100.0;
    double quality = std::clamp(normalizedQuality, 0.0, 100.0);

    return executionResult(quality);
}

void MaxErrorCheck::validateParameters() const
{
    if (maxMaxError <= 0.0)
        throw std::invalid_argument("maxMaxError must be positive");
}

std::string MaxErrorCheck::getMessageForSeverity(EvaluationSeverity severity) const
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