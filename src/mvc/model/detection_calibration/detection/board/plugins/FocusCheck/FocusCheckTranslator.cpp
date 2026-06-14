#include "FocusCheckTranslator.hpp"
#include <unordered_map>
#include <parameter/ParameterInfo.hpp>
#include "FocusCheck.hpp"


void FocusCheckTranslator::applyParametersToPlugin(
	const std::shared_ptr<IPlugin>& plugin,
	const std::vector<std::shared_ptr<ParameterInfo>>& parameters
)
{
	if (!plugin)
		throw std::invalid_argument("plugin is nullptr");

	if (parameters.empty())
		return;

	std::shared_ptr<FocusCheck> focusPlugin = std::dynamic_pointer_cast<FocusCheck>(plugin);
	if (!focusPlugin)
		throw std::invalid_argument("plugin is not of type FocusCheck");

	std::unordered_map<std::string, std::shared_ptr<ParameterInfo>> parameterMap;
	for (const auto& param : parameters) parameterMap[param->name()] = param;


    // Minimum marks required
    {
        auto it = parameterMap.find("min_marks_count");
        if (it != parameterMap.end())
        {
            int newValue = it->second->getValue<int>();
            if (newValue != focusPlugin->minMarksCount)
            {
                focusPlugin->minMarksCount = newValue;
            }
        }
    }

    // Mark dilation radius
    {
        auto it = parameterMap.find("mark_dilation_radius");
        if (it != parameterMap.end())
        {
            double newValue = it->second->getValue<double>();
            if (newValue != focusPlugin->markDilationRadius)
            {
                focusPlugin->markDilationRadius = newValue;
            }
        }
    }

    // Sigma
    {
        auto it = parameterMap.find("sigma");
        if (it != parameterMap.end())
        {
            double newValue = it->second->getValue<double>();
            if (newValue != focusPlugin->sigma)
            {
                focusPlugin->sigma = newValue;
            }
        }
    }

    // Gradient scaling factor
    {
        auto it = parameterMap.find("gradient_scaling_factor");
        if (it != parameterMap.end())
        {
            double newValue = it->second->getValue<double>();
            if (newValue != focusPlugin->gradientScalingFactor)
            {
                focusPlugin->gradientScalingFactor = newValue;
            }
        }
    }

    // Gradient percentile divisor
    {
        auto it = parameterMap.find("gradient_percentile_divisor");
        if (it != parameterMap.end())
        {
            double newValue = it->second->getValue<double>();
            if (newValue != focusPlugin->gradientPercentileDivisor)
            {
                focusPlugin->gradientPercentileDivisor = newValue;
            }
        }
    }

    // Minimum score offset
    {
        auto it = parameterMap.find("min_score_offset");
        if (it != parameterMap.end())
        {
            double newValue = it->second->getValue<double>();
            if (newValue != focusPlugin->minScoreOffset)
            {
                focusPlugin->minScoreOffset = newValue;
            }
        }
    }
}
