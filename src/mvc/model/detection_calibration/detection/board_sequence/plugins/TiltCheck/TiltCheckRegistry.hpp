#pragma once
#include <parameter/ParameterRegistry.hpp>


/**
 * @brief Registry for managing tilt angles validation parameters.
 */
class TiltCheckRegistry : public ParameterRegistry
{
public:
	/** @brief Category string used for grouping these parameters. */
	static const std::string CATEGORY;

	TiltCheckRegistry();

	~TiltCheckRegistry() = default;

private:
	void registerAllParameters();
};