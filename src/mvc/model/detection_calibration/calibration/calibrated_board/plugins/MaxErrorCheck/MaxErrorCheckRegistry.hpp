#pragma once
#include <parameter/ParameterRegistry.hpp>


/**
 * @brief Registry for managing max reprojection error validation parameters.
 */
class MaxErrorCheckRegistry : public ParameterRegistry
{
public:
	/** @brief Category string used for grouping these parameters. */
	static const std::string CATEGORY;

	MaxErrorCheckRegistry();

	~MaxErrorCheckRegistry() = default;

private:
	void registerAllParameters();
};