#pragma once
#include <parameter/ParameterRegistry.hpp>


/**
 * @brief Registry for managing (FOV) coverage validation parameters.
 */
class ContrastCheckRegistry : public ParameterRegistry
{
public:
	/** @brief Category string used for grouping these parameters. */
	static const std::string CATEGORY;

	ContrastCheckRegistry();

	~ContrastCheckRegistry() = default;

private:
	void registerAllParameters();
};