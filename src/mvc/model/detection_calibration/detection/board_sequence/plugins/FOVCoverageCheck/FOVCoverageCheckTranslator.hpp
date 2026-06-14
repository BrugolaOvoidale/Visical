#pragma once
#include <evaluator/ParameterToPluginTranslator.hpp>


/**
 * @brief Specialized translator for Field of View (FOV) coverage check plugins.
 */
class FOVCoverageCheckTranslator : public ParameterToPluginTranslator
{
public:
	FOVCoverageCheckTranslator() = default;

	~FOVCoverageCheckTranslator() = default;

	void applyParametersToPlugin(
		const std::shared_ptr<IPlugin>& plugin,
		const std::vector<std::shared_ptr<ParameterInfo>>& parameters
	) override;
};