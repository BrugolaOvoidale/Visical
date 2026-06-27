#include "ImagesCountCheck.hpp"


ImagesCountCheck::ImagesCountCheck(double threshold)
    : Plugin(
        std::string(ID),
        std::string(NAME),
        std::string(DESCRIPTION),
        {},
        threshold)
{
}

/////////////////////////////////////////////////////////////

std::shared_ptr<ImagesCountCheck> ImagesCountCheck::create(double threshold)
{
    struct Enabler : public ImagesCountCheck
    {
        Enabler(double thr) : ImagesCountCheck(thr) {}
    };

    return std::make_shared<Enabler>(threshold);
}

/////////////////////////////////////////////////////////////

std::shared_ptr<PluginResult> ImagesCountCheck::executeImpl(
    const std::vector<std::shared_ptr<Board>>& boards,
    const std::unordered_map<std::string, std::shared_ptr<PluginResult>>& producersResults) const
{
    double quality = std::min(100.0, (boards.size() * 100.0) / minImagesCount);

    return executionResult(quality);
}

void ImagesCountCheck::validateParameters() const
{
    if (minImagesCount == 0)
        throw std::runtime_error("minImagesCount must be greater than 0");
}

std::string ImagesCountCheck::getMessageForSeverity(EvaluationSeverity severity) const
{
    switch (severity)
    {
    case EvaluationSeverity::FAILED:
        return "Images count check failed";
    case EvaluationSeverity::CRITICAL:
        return "Number of images is too low";
    case EvaluationSeverity::WARNING:
        return "Number of images is low";
    case EvaluationSeverity::OK:
    default:
        return "";
    }
}