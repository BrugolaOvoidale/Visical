#include "ContrastCheckTranslator.hpp"
#include <unordered_map>
#include <parameter/ParameterInfo.hpp>
#include "ContrastCheck.hpp"


void ContrastCheckTranslator::applyParametersToPlugin(
	const std::shared_ptr<IPlugin>& plugin,
	const std::vector<std::shared_ptr<ParameterInfo>>& parameters
)
{
	if (!plugin)
		throw std::invalid_argument("plugin is nullptr");

	if (parameters.empty())
		return;

	std::shared_ptr<ContrastCheck> contrastPlugin = std::dynamic_pointer_cast<ContrastCheck>(plugin);
	if (!contrastPlugin)
		throw std::invalid_argument("plugin is not of type ContrastCheck");

	std::unordered_map<std::string, std::shared_ptr<ParameterInfo>> parameterMap;
	for (const auto& param : parameters) parameterMap[param->name()] = param;


    // Minimum detectable contrast
    {
        auto it = parameterMap.find("min_detectable_contrast");
        if (it != parameterMap.end())
        {
            int newValue = it->second->getValue<int>();
            if (newValue != contrastPlugin->minDetectableContrast)
            {
                contrastPlugin->minDetectableContrast = newValue;
            }
        }
    }

    // Sufficient contrast threshold
    {
        auto it = parameterMap.find("sufficient_contrast_threshold");
        if (it != parameterMap.end())
        {
            int newValue = it->second->getValue<int>();
            if (newValue != contrastPlugin->sufficientContrastThr)
            {
                contrastPlugin->sufficientContrastThr = newValue;
            }
        }
    }

    // Mark dilation radius
    {
        auto it = parameterMap.find("mark_dilation_radius");
        if (it != parameterMap.end())
        {
            double newValue = it->second->getValue<double>();
            if (newValue != contrastPlugin->markDilationRadius)
            {
                contrastPlugin->markDilationRadius = newValue;
            }
        }
    }
}
