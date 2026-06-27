#pragma once
#include <parameter/ParameterRegistry.hpp>


/**
 * @brief Registry for managing detection parameters for ArUco-based patterns.
 */
class ArucoParametersRegistry : public ParameterRegistry
{
public:
	static const std::string& CATEGORY();
	static const std::string& CATEGORY_GEOMETRY();
	static const std::string& CATEGORY_DETECTION();
	static const std::string& CATEGORY_REFINE();

	ArucoParametersRegistry();

	~ArucoParametersRegistry() = default;

private:
	void registerAllParameters();
};