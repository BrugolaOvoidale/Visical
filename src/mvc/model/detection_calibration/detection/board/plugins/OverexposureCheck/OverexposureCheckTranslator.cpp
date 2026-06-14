#include "OverexposureCheckTranslator.hpp"
#include <unordered_map>
#include <parameter/ParameterInfo.hpp>
#include "OverexposureCheck.hpp"


void OverexposureCheckTranslator::applyParametersToPlugin(
	const std::shared_ptr<IPlugin>& plugin,
	const std::vector<std::shared_ptr<ParameterInfo>>& parameters
)
{
	if (!plugin)
		throw std::invalid_argument("plugin is nullptr");

	if (parameters.empty())
		return;

	std::shared_ptr<OverexposureCheck> overexposurePlugin = std::dynamic_pointer_cast<OverexposureCheck>(plugin);
	if (!overexposurePlugin)
		throw std::invalid_argument("plugin is not of type OverexposureCheck");

	std::unordered_map<std::string, std::shared_ptr<ParameterInfo>> parameterMap;
	for (const auto& param : parameters) parameterMap[param->name()] = param;


    // Overexposure lower bound
    {
        auto it = parameterMap.find("min_threshold");
        if (it != parameterMap.end())
        {
            int newValue = it->second->getValue<int>();
            if (newValue != overexposurePlugin->minThreshold)
            {
                overexposurePlugin->minThreshold = newValue;
            }
        }
    }

    // Overexposure upper bound
    {
        auto it = parameterMap.find("max_threshold");
        if (it != parameterMap.end())
        {
            int newValue = it->second->getValue<int>();
            if (newValue != overexposurePlugin->maxThreshold)
            {
                overexposurePlugin->maxThreshold = newValue;
            }
        }
    }

    // Maximum allowed overexposed area
    {
        auto it = parameterMap.find("max_area_ratio");
        if (it != parameterMap.end())
        {
            double newValue = it->second->getValue<double>();
            if (newValue != overexposurePlugin->maxAreaRatio)
            {
                overexposurePlugin->maxAreaRatio = newValue;
            }
        }
    }
}
