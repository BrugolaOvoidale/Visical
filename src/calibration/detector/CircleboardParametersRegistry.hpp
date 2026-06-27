#pragma once
#include <parameter/ParameterRegistry.hpp>


/**
 * @brief Registry for managing detection parameters for circleboard pattern.
 */
class CircleboardParametersRegistry : public ParameterRegistry
{
public:
	static const std::string CATEGORY;
	static const std::string CATEGORY_GEOMETRY;
	static const std::string CATEGORY_DETECTION;

	CircleboardParametersRegistry();

	~CircleboardParametersRegistry() = default;

private:
	void registerAllParameters();
};