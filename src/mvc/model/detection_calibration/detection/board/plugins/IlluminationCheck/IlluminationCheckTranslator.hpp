#pragma once
#include <evaluator/ParameterToPluginTranslator.hpp>


/**
 * @brief Specialized translator for illumination homogeneity check plugins.
 */
class IlluminationCheckTranslator : public ParameterToPluginTranslator
{
public:
	IlluminationCheckTranslator() = default;

	~IlluminationCheckTranslator() = default;

	void applyParametersToPlugin(
		const std::shared_ptr<IPlugin>& plugin,
		const std::vector<std::shared_ptr<ParameterInfo>>& parameters
	) override;
};