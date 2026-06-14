#include "FOVCoverageCheckTranslator.hpp"
#include <unordered_map>
#include <parameter/ParameterInfo.hpp>
#include "FOVCoverageCheck.hpp"


void FOVCoverageCheckTranslator::applyParametersToPlugin(
	const std::shared_ptr<IPlugin>& plugin,
	const std::vector<std::shared_ptr<ParameterInfo>>& parameters
)
{
	if (!plugin)
		throw std::invalid_argument("plugin is nullptr");

	if (parameters.empty())
		return;

	std::shared_ptr<FOVCoverageCheck> fovPlugin = std::dynamic_pointer_cast<FOVCoverageCheck>(plugin);
	if (!fovPlugin)
		throw std::invalid_argument("plugin is not of type FOVCoverageCheck");

	std::unordered_map<std::string, std::shared_ptr<ParameterInfo>> parameterMap;
	for (const auto& param : parameters) parameterMap[param->name()] = param;


    // Border exclusion margin
    {
        auto it = parameterMap.find("border_exclusion_ratio");
        if (it != parameterMap.end())
        {
            double newValue = it->second->getValue<double>();
            if (newValue != fovPlugin->borderExclusionRatio)
            {
                fovPlugin->borderExclusionRatio = newValue;
            }
        }
    }

    // Coverage sensitivity
    {
        auto it = parameterMap.find("coverage_normalization_factor");
        if (it != parameterMap.end())
        {
            double newValue = it->second->getValue<double>();
            if (newValue != fovPlugin->coverageNormalizationFactor)
            {
                fovPlugin->coverageNormalizationFactor = newValue;
            }
        }
    }

    // Minimum acceptable coverage
    {
        auto it = parameterMap.find("min_coverage_threshold");
        if (it != parameterMap.end())
        {
            double newValue = it->second->getValue<double>();
            if (newValue != fovPlugin->minCoverageThreshold)
            {
                fovPlugin->minCoverageThreshold = newValue;
            }
        }
    }

    // Maximum acceptable coverage
    {
        auto it = parameterMap.find("max_coverage_threshold");
        if (it != parameterMap.end())
        {
            double newValue = it->second->getValue<double>();
            if (newValue != fovPlugin->maxCoverageThreshold)
            {
                fovPlugin->maxCoverageThreshold = newValue;
            }
        }
    }
}
