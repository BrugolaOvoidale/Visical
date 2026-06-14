#include "BoardSizeCheckTranslator.hpp"
#include <unordered_map>
#include <parameter/ParameterInfo.hpp>
#include "BoardSizeCheck.hpp"


void BoardSizeCheckTranslator::applyParametersToPlugin(
	const std::shared_ptr<IPlugin>& plugin,
	const std::vector<std::shared_ptr<ParameterInfo>>& parameters
)
{
	if (!plugin)
		throw std::invalid_argument("plugin is nullptr");

	if (parameters.empty())
		return;

	std::shared_ptr<BoardSizeCheck> boardSizePlugin = std::dynamic_pointer_cast<BoardSizeCheck>(plugin);
	if (!boardSizePlugin)
		throw std::invalid_argument("plugin is not of type BoardSizeCheck");

	std::unordered_map<std::string, std::shared_ptr<ParameterInfo>> parameterMap;
	for (const auto& param : parameters) parameterMap[param->name()] = param;


	// Expected coverage
	{
		auto it = parameterMap.find("expected_coverage");
		if (it != parameterMap.end())
		{
			double newValue = it->second->getValue<double>();
			if (newValue != boardSizePlugin->expectedCoverage)
			{
				boardSizePlugin->expectedCoverage = newValue;
			}
		}
	}
}
