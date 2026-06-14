#pragma once
#include <evaluator/ParameterToPluginTranslator.hpp>


/**
 * @brief Specialized translator for board size plugins.
 */
class BoardSizeCheckTranslator : public ParameterToPluginTranslator
{
public:
	BoardSizeCheckTranslator() = default;

	~BoardSizeCheckTranslator() = default;

	void applyParametersToPlugin(
		const std::shared_ptr<IPlugin>& plugin,
		const std::vector<std::shared_ptr<ParameterInfo>>& parameters
	) override;
};