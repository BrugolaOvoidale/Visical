#include "IlluminationCheckTranslator.hpp"
#include <unordered_map>
#include <parameter/ParameterInfo.hpp>
#include "IlluminationCheck.hpp"


void IlluminationCheckTranslator::applyParametersToPlugin(
	const std::shared_ptr<IPlugin>& plugin,
	const std::vector<std::shared_ptr<ParameterInfo>>& parameters
)
{
	if (!plugin)
		throw std::invalid_argument("plugin is nullptr");

	if (parameters.empty())
		return;

	std::shared_ptr<IlluminationCheck> illuminationPlugin = std::dynamic_pointer_cast<IlluminationCheck>(plugin);
	if (!illuminationPlugin)
		throw std::invalid_argument("plugin is not of type IlluminationCheck");

	std::unordered_map<std::string, std::shared_ptr<ParameterInfo>> parameterMap;
	for (const auto& param : parameters) parameterMap[param->name()] = param;


    // Mark dilation radius
    {
        auto it = parameterMap.find("mark_dilation_radius");
        if (it != parameterMap.end())
        {
            double newValue = it->second->getValue<double>();
            if (newValue != illuminationPlugin->markDilationRadius)
            {
                illuminationPlugin->markDilationRadius = newValue;
            }
        }
    }

    // Intensity deviation scale
    {
        auto it = parameterMap.find("intensity_deviation_scale");
        if (it != parameterMap.end())
        {
            double newValue = it->second->getValue<double>();
            if (newValue != illuminationPlugin->intensityDeviationScale)
            {
                illuminationPlugin->intensityDeviationScale = newValue;
            }
        }
    }

    // Score offset
    {
        auto it = parameterMap.find("score_offset_factor");
        if (it != parameterMap.end())
        {
            double newValue = it->second->getValue<double>();
            if (newValue != illuminationPlugin->scoreOffsetFactor)
            {
                illuminationPlugin->scoreOffsetFactor = newValue;
            }
        }
    }
}
