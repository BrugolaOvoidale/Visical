#pragma once
#include <parameter/ParameterRegistry.hpp>


/**
 * @brief Registry for managing detection parameters for AprilTag pattern.
 */
class AprilTagParametersRegistry : public ParameterRegistry
{
public:
	static const std::string CATEGORY;
	static const std::string CATEGORY_GEOMETRY;
	static const std::string CATEGORY_DETECTION;

	AprilTagParametersRegistry();

	~AprilTagParametersRegistry() = default;

private:
	void registerAllParameters();
};