#pragma once
#include <parameter/ParameterRegistry.hpp>


/**
 * @brief Registry for managing detection Model parameters.
 */
class DetectionModelRegistry : public ParameterRegistry
{
public:
	static inline const std::string CATEGORY_PRE_PROC { "pre_processing" };

	DetectionModelRegistry();

	~DetectionModelRegistry() = default;

private:
	void registerAllParameters();
};