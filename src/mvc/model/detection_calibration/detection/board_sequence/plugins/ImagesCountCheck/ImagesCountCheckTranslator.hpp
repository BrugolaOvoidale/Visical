#pragma once
#include <evaluator/ParameterToPluginTranslator.hpp>


/**
 * @brief Specialized translator for images count check plugins.
 */
class ImagesCountCheckTranslator : public ParameterToPluginTranslator
{
public:
	ImagesCountCheckTranslator() = default;

	~ImagesCountCheckTranslator() = default;

	void applyParametersToPlugin(
		const std::shared_ptr<IPlugin>& plugin,
		const std::vector<std::shared_ptr<ParameterInfo>>& parameters
	) override;
};