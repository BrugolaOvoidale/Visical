#pragma once
#include <parameter/ParameterRegistry.hpp>


/**
 * @brief Registry for managing images count validation parameters.
 */
class ImagesCountCheckRegistry : public ParameterRegistry
{
public:
	/** @brief Category string used for grouping these parameters. */
	static const std::string CATEGORY;

	ImagesCountCheckRegistry();

	~ImagesCountCheckRegistry() = default;

private:
	void registerAllParameters();
};