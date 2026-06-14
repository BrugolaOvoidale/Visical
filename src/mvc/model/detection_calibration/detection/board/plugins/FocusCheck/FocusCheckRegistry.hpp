#pragma once
#include <parameter/ParameterRegistry.hpp>


/**
 * @brief Registry for managing marks focus validation parameters.
 */
class FocusCheckRegistry : public ParameterRegistry
{
public:
	/** @brief Category string used for grouping these parameters. */

	static const std::string CATEGORY;

	FocusCheckRegistry();

	~FocusCheckRegistry() = default;

private:
	void registerAllParameters();
};