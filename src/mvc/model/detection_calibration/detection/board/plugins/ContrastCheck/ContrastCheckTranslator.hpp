#pragma once
#include <evaluator/ParameterToPluginTranslator.hpp>


/**
 * @brief Specialized translator for marks contrast check plugins.
 */
class ContrastCheckTranslator : public ParameterToPluginTranslator
{
public:
	ContrastCheckTranslator() = default;

	~ContrastCheckTranslator() = default;

	void applyParametersToPlugin(
		const std::shared_ptr<IPlugin>& plugin,
		const std::vector<std::shared_ptr<ParameterInfo>>& parameters
	) override;
};