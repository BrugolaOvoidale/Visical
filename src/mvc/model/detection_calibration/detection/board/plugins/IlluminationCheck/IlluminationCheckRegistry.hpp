#pragma once
#include <parameter/ParameterRegistry.hpp>


/**
 * @brief Registry for managing illumination homogeneity validation parameters.
 */
class IlluminationCheckRegistry : public ParameterRegistry
{
public:
	/** @brief Category string used for grouping these parameters. */
	static const std::string CATEGORY;

	IlluminationCheckRegistry();

	~IlluminationCheckRegistry() = default;

private:
	void registerAllParameters();
};