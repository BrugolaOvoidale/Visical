#pragma once
#include <parameter/ParameterRegistry.hpp>


/**
 * @brief Registry for managing detection parameters for circleboard pattern.
 */
class CircleboardParamsRegistry : public ParameterRegistry
{
public:
	static const std::string CATEGORY;

	CircleboardParamsRegistry();

	~CircleboardParamsRegistry() = default;

private:
	void registerAllParameters();
};