#pragma once
#include <parameter/ParameterRegistry.hpp>


/**
 * @brief Registry for managing board size validation parameters.
 */
class BoardSizeCheckRegistry : public ParameterRegistry
{
public:
	/** @brief Category string used for grouping these parameters. */
	static const std::string CATEGORY;

	BoardSizeCheckRegistry();

	~BoardSizeCheckRegistry() = default;

private:
	void registerAllParameters();
};