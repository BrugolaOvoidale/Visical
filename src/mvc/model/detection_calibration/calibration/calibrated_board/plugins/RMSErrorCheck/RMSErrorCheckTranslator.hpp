#pragma once
#include <evaluator/ParameterToPluginTranslator.hpp>


/**
 * @brief Specialized translator for RMS error check plugins.
 */
class RMSErrorCheckTranslator : public ParameterToPluginTranslator
{
public:
	RMSErrorCheckTranslator() = default;

	~RMSErrorCheckTranslator() = default;

	void applyParametersToPlugin(
		const std::shared_ptr<IPlugin>& plugin,
		const std::vector<std::shared_ptr<ParameterInfo>>& parameters
	) override;
};