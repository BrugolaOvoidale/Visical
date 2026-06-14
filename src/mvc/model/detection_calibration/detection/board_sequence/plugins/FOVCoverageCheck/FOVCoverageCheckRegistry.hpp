#pragma once
#include <parameter/ParameterRegistry.hpp>


/**
 * @brief Registry for managing (FOV) coverage validation parameters.
 */
class FOVCoverageCheckRegistry : public ParameterRegistry
{
public:
	/** @brief Category string used for grouping these parameters. */
	static const std::string CATEGORY;

	FOVCoverageCheckRegistry();

	~FOVCoverageCheckRegistry() = default;

private:
	void registerAllParameters();
};