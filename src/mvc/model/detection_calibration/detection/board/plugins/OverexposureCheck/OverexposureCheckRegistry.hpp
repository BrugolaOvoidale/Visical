#pragma once
#include <parameter/ParameterRegistry.hpp>


/**
 * @brief Registry for managing overexposure validation parameters.
 */
class OverexposureCheckRegistry : public ParameterRegistry
{
public:
	/** @brief Category string used for grouping these parameters. */
	static const std::string CATEGORY;

	OverexposureCheckRegistry();
	
	~OverexposureCheckRegistry() = default;

private:
	void registerAllParameters();
};