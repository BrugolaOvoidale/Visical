#pragma once
#include <parameter/ParameterRegistry.hpp>


/**
 * @brief Registry for managing common parameters for claibration board detection.
 */
class PatternParametersRegistry : public ParameterRegistry
{
public:
	/** * @brief The category identifier used for grouping these parameters in the system. */
	static const std::string& CATEGORY();

	PatternParametersRegistry();

	~PatternParametersRegistry() = default;

private:
	void registerAllParameters();
};