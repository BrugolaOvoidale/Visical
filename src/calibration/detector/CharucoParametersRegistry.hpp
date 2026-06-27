#pragma once
#include <parameter/ParameterRegistry.hpp>


/**
 * @brief Registry for managing detection parameters for ChArUco pattern.
 */
class CharucoParametersRegistry : public ParameterRegistry
{
public:
	static const std::string CATEGORY;
	static const std::string CATEGORY_GEOMETRY;
	static const std::string CATEGORY_DETECTION;

	CharucoParametersRegistry();

	~CharucoParametersRegistry() = default;

private:
	void registerAllParameters();
};