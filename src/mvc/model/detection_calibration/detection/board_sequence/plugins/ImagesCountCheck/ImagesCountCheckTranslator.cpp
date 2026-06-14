#include "ImagesCountCheckTranslator.hpp"
#include <unordered_map>
#include <parameter/ParameterInfo.hpp>
#include "ImagesCountCheck.hpp"


void ImagesCountCheckTranslator::applyParametersToPlugin(
	const std::shared_ptr<IPlugin>& plugin,
	const std::vector<std::shared_ptr<ParameterInfo>>& parameters
)
{
	if (!plugin)
		throw std::invalid_argument("plugin is nullptr");

	if (parameters.empty())
		return;

	std::shared_ptr<ImagesCountCheck> imgCountPlugin = std::dynamic_pointer_cast<ImagesCountCheck>(plugin);
	if (!imgCountPlugin)
		throw std::invalid_argument("plugin is not of type ImagesCountCheck");

	std::unordered_map<std::string, std::shared_ptr<ParameterInfo>> parameterMap;
	for (const auto& param : parameters) parameterMap[param->name()] = param;


	// Minimum image count
	{
		auto it = parameterMap.find("min_image_count");
		if (it != parameterMap.end())
		{
			int newValue = it->second->getValue<int>();
			if (newValue != imgCountPlugin->minImagesCount)
			{
				imgCountPlugin->minImagesCount = newValue;
			}
		}
	}
}
