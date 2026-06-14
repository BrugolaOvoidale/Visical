#pragma once
#include <parameter/ParameterRegistry.hpp>


/**
 * @brief Registry for lens distortion model parameters.
 *
 * This class handles the registration and categorization of parameters used to
 * model various types of optical distortion.
 */
class DistortionModelRegistry : public ParameterRegistry
{
public:
    /** @brief Root category name for all distortion models. */
    static const std::string CATEGORY;

    /** @brief Category for standard Brown-Conrady distortion parameters. */
    static const std::string CATEGORY_STANDARD;

    /** @brief Category for higher-order rational polynomial distortion parameters. */
    static const std::string CATEGORY_RATIONAL;

    /** @brief Category for thin prism distortion parameters (tangential and radial). */
    static const std::string CATEGORY_THIN_PRISM;

    /** @brief Category for tilted sensor/lens plane distortion parameters. */
    static const std::string CATEGORY_TILTED;

public:
	DistortionModelRegistry();

private:
	void registerAllParameters();
};