#pragma once
#include <evaluator/ParameterToPluginTranslator.hpp>


/**
 * @brief Specialized translator for board overexposure check plugins.
 */
class OverexposureCheckTranslator : public ParameterToPluginTranslator
{
public:
	OverexposureCheckTranslator() = default;

	~OverexposureCheckTranslator() = default;

	void applyParametersToPlugin(
		const std::shared_ptr<IPlugin>& plugin,
		const std::vector<std::shared_ptr<ParameterInfo>>& parameters
	) override;
};