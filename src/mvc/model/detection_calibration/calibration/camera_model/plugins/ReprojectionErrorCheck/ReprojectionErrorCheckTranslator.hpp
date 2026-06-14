#pragma once
#include <evaluator/ParameterToPluginTranslator.hpp>


/**
 * @brief Specialized translator for max reprojection error check plugins.
 */
class ReprojectionErrorCheckTranslator : public ParameterToPluginTranslator
{
public:
	ReprojectionErrorCheckTranslator() = default;

	~ReprojectionErrorCheckTranslator() = default;

	void applyParametersToPlugin(
		const std::shared_ptr<IPlugin>& plugin,
		const std::vector<std::shared_ptr<ParameterInfo>>& parameters
	) override;
};