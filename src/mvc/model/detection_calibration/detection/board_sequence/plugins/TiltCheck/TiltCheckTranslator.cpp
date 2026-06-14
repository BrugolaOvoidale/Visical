#include "TiltCheckTranslator.hpp"
#include <unordered_map>
#include <parameter/ParameterInfo.hpp>
#include "TiltCheck.hpp"


void TiltCheckTranslator::applyParametersToPlugin(
	const std::shared_ptr<IPlugin>& plugin,
	const std::vector<std::shared_ptr<ParameterInfo>>& parameters
)
{
	if (!plugin)
		throw std::invalid_argument("plugin is nullptr");

	if (parameters.empty())
		return;

	std::shared_ptr<TiltCheck> tiltPlugin = std::dynamic_pointer_cast<TiltCheck>(plugin);
	if (!tiltPlugin)
		throw std::invalid_argument("plugin is not of type TiltCheck");

	std::unordered_map<std::string, std::shared_ptr<ParameterInfo>> parameterMap;
	for (const auto& param : parameters) parameterMap[param->name()] = param;


    // Maximum angle cap
    {
        auto it = parameterMap.find("max_angle_cap");
        if (it != parameterMap.end())
        {
            double newValue = it->second->getValue<double>();
            if (newValue != tiltPlugin->maxAngleCap)
            {
                tiltPlugin->maxAngleCap = newValue;
            }
        }
    }

    // Angle scaling target
    {
        auto it = parameterMap.find("angle_scaling_target");
        if (it != parameterMap.end())
        {
            double newValue = it->second->getValue<double>();
            if (newValue != tiltPlugin->angleScalingTarget)
            {
                tiltPlugin->angleScalingTarget = newValue;
            }
        }
    }

    // Score exponent
    {
        auto it = parameterMap.find("score_exponent");
        if (it != parameterMap.end())
        {
            double newValue = it->second->getValue<double>();
            if (newValue != tiltPlugin->scoreExponent)
            {
                tiltPlugin->scoreExponent = newValue;
            }
        }
    }

    // Minimum images for scoring
    {
        auto it = parameterMap.find("min_image_count");
        if (it != parameterMap.end())
        {
            int newValue = it->second->getValue<int>();
            if (newValue != tiltPlugin->minImages)
            {
                tiltPlugin->minImages = newValue;
            }
        }
    }

    // Score adjustment multiplier
    {
        auto it = parameterMap.find("score_adjustment_multiplier");
        if (it != parameterMap.end())
        {
            double newValue = it->second->getValue<double>();
            if (newValue != tiltPlugin->scoreAdjustmentMultiplier)
            {
                tiltPlugin->scoreAdjustmentMultiplier = newValue;
            }
        }
    }

    // Slant weight
    {
        auto it = parameterMap.find("slant_weight");
        if (it != parameterMap.end())
        {
            double newValue = it->second->getValue<double>();
            if (newValue != tiltPlugin->slantWeight)
            {
                tiltPlugin->slantWeight = newValue;
            }
        }
    }

    // Pan weight
    {
        auto it = parameterMap.find("pan_weight");
        if (it != parameterMap.end())
        {
            double newValue = it->second->getValue<double>();
            if (newValue != tiltPlugin->panWeight)
            {
                tiltPlugin->panWeight = newValue;
            }
        }
    }
}
