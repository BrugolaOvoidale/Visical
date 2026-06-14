#pragma once
#include <evaluator/ParameterToPluginTranslator.hpp>


/**
 * @brief Specialized translator for max reprojection error check plugins.
 */
class MaxErrorCheckTranslator : public ParameterToPluginTranslator
{
public:
	MaxErrorCheckTranslator() = default;

	~MaxErrorCheckTranslator() = default;

	void applyParametersToPlugin(
		const std::shared_ptr<IPlugin>& plugin,
		const std::vector<std::shared_ptr<ParameterInfo>>& parameters
	) override;
};