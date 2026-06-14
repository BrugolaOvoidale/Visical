#pragma once
#include <parameter/ParameterRegistry.hpp>


/**
 * @brief Registry for managing calibration parameters.
 */
class CalibrationParametersRegistry : public ParameterRegistry
{
public:
	static inline const std::string CATEGORY{ "calibration" };

	CalibrationParametersRegistry();

	~CalibrationParametersRegistry() = default;

private:
	void registerAllParameters();
};