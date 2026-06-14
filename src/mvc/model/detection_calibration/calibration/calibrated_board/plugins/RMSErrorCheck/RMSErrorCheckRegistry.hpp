#pragma once
#include <parameter/ParameterRegistry.hpp>


/**
 * @brief Registry for managing RMS error validation parameters.
 */
class RMSErrorCheckRegistry : public ParameterRegistry
{
public:
	/** @brief Category string used for grouping these parameters. */
	static const std::string CATEGORY;

	RMSErrorCheckRegistry();

	~RMSErrorCheckRegistry() = default;

private:
	void registerAllParameters();
};