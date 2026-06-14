#pragma once
#include <parameter/ParameterRegistry.hpp>


/**
 * @brief Registry for managing camera internal parameters (intrinsics).
 */
class CameraIntrinsicsRegistry : public ParameterRegistry
{
public:
	/** * @brief The category identifier used for grouping these parameters in the system. */
	static inline const std::string CATEGORY{ "camera" };

	CameraIntrinsicsRegistry();

	~CameraIntrinsicsRegistry() = default;

private:
	void registerAllParameters();
};