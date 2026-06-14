#pragma once
#include <parameter/ParameterRegistry.hpp>


/**
 * @brief Registry for managing common parameters for claibration board detection.
 */
class PatternParamsRegistry : public ParameterRegistry
{
public:
	/** * @brief The category identifier used for grouping these parameters in the system. */
	static const std::string& CATEGORY();

	PatternParamsRegistry();

	~PatternParamsRegistry() = default;

private:
	void registerAllParameters();
};