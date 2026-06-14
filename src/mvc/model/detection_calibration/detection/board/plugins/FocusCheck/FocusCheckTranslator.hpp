#pragma once
#include <evaluator/ParameterToPluginTranslator.hpp>


/**
 * @brief Specialized translator marks focus check plugins.
 */
class FocusCheckTranslator : public ParameterToPluginTranslator
{
public:

	FocusCheckTranslator() = default;

	~FocusCheckTranslator() = default;

	void applyParametersToPlugin(
		const std::shared_ptr<IPlugin>& plugin,
		const std::vector<std::shared_ptr<ParameterInfo>>& parameters
	) override;
};