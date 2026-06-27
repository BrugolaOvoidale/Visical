#include "ReprojectionErrorCheck.hpp"
#include <stdexcept>


ReprojectionErrorCheck::ReprojectionErrorCheck(double threshold)
    : Plugin(
        std::string(ID),
        std::string(NAME),
        std::string(DESCRIPTION),
        {},
        threshold)
{
}

/////////////////////////////////////////////////////////////

std::shared_ptr<ReprojectionErrorCheck> ReprojectionErrorCheck::create(double threshold)
{
    struct Enabler : public ReprojectionErrorCheck
    {
        Enabler(double thr) : ReprojectionErrorCheck(thr) {}
    };

    return std::make_shared<Enabler>(threshold);
}

/////////////////////////////////////////////////////////////

std::shared_ptr<PluginResult> ReprojectionErrorCheck::executeImpl(
    const CameraModel& camModel,
    const std::unordered_map<std::string, std::shared_ptr<PluginResult>>& producersResults) const
{
    const double reprojError = camModel.reprojectionError();

    double normalizedQuality = (1.0 - (reprojError / maxError)) * 100.0;
    double quality = std::clamp(normalizedQuality, 0.0, 100.0);

    return executionResult(quality);
}

void ReprojectionErrorCheck::validateParameters() const
{
    if (maxError <= 0.0)
        throw std::invalid_argument("maxError must be positive");
}

std::string ReprojectionErrorCheck::getMessageForSeverity(EvaluationSeverity severity) const
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