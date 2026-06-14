#include "RMSErrorCheckTranslator.hpp"
#include <unordered_map>
#include <stdexcept>
#include <parameter/ParameterInfo.hpp>
#include "RMSErrorCheck.hpp"


void RMSErrorCheckTranslator::applyParametersToPlugin(
	const std::shared_ptr<IPlugin>& plugin,
	const std::vector<std::shared_ptr<ParameterInfo>>& parameters
)
{
	if (!plugin)
		throw std::invalid_argument("plugin is nullptr");

	if (parameters.empty())
		return;

	std::shared_ptr<RMSErrorCheck> reprojErrorPlugin = std::dynamic_pointer_cast<RMSErrorCheck>(plugin);
	if (!reprojErrorPlugin)
		throw std::invalid_argument("plugin is not of type RMSErrorCheck");

	std::unordered_map<std::string, std::shared_ptr<ParameterInfo>> parameterMap;
	for (const auto& param : parameters) parameterMap[param->name()] = param;


	// Expected coverage
	{
		auto it = parameterMap.find("max_rms_error");
		if (it != parameterMap.end())
		{
			double newValue = it->second->getValue<double>();
			if (newValue != reprojErrorPlugin->maxRMSError)
			{
				reprojErrorPlugin->maxRMSError = newValue;
			}
		}
	}
}
