#include <UtilityFunctions.hpp>
#include <parameter/enum/ParameterEnum.hpp>
#include <parameter/numeric/ParameterNumeric.hpp>
#include <parameter/bool/ParameterBool.hpp>
#include "CalibrationParametersRegistry.hpp"
#include "CalibrationParameters.hpp"


CalibrationParametersRegistry::CalibrationParametersRegistry()
	: ParameterRegistry()
{
	registerAllParameters();
}

/////////////////////////////////////////////////////////////

void CalibrationParametersRegistry::registerAllParameters()
{
	// Solver type
	{
		std::shared_ptr<Parameter> aParam = ParameterEnum::create(
			"solver_type",
			CATEGORY,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			UtilityFunctions::enumValuesToIntMap<CalibrationParameters::SolverType>(),
			UtilityFunctions::stringFromEnum(CalibrationParameters::Defaults::solver).value(),
			"Solver type",
			std::nullopt,
			"Specifies the numerical method used to solve the calibration optimization problem.\n"
			"OpenCV supports different matrix decomposition methods:\n\n"
			"- SVD: Most stable and accurate, but slower.\n"
			"- QR: Faster than SVD, with slightly reduced precision.\n"
			"- LU: Fastest option, but may be less stable and less precise."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}

	// Principal point mode
	{
		std::shared_ptr<Parameter> aParam = ParameterEnum::create(
			"principal_point_mode",
			CATEGORY,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			UtilityFunctions::enumValuesToIntMap<CalibrationParameters::PrincipalPointMode>(),
			UtilityFunctions::stringFromEnum(CalibrationParameters::Defaults::ppMode).value(),
			"Principal point mode",
			std::nullopt,
			"Controls how the camera principal point is handled during calibration. In FIX_AT_INITIAL mode, the principal point(cx, cy) is locked to the values provided by the user. This requires 'Use initial guess' to be enabled; otherwise OpenCV will ignore the user values and use its own estimate"
		);

		registerParameter(
			aParam->getParameterView()
		);
	}

	// Focal length mode
	{
		std::shared_ptr<Parameter> aParam = ParameterEnum::create(
			"focal_length_mode",
			CATEGORY,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			UtilityFunctions::enumValuesToIntMap<CalibrationParameters::FocalLengthMode>(),
			UtilityFunctions::stringFromEnum(CalibrationParameters::Defaults::focalMode).value(),
			"Focal length mode",
			std::nullopt,
			"Controls how the focal lengths (fx, fy) are handled during calibration. In FIX_BOTH mode, fx and fy are locked to the values provided by the user. This requires 'Use initial guess' to be enabled; otherwise OpenCV will ignore the user values and use its own estimated focal lengths."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}

	// Tangential distortion mode
	{
		std::shared_ptr<Parameter> aParam = ParameterEnum::create(
			"tangential_distortion_mode",
			CATEGORY,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			UtilityFunctions::enumValuesToIntMap<CalibrationParameters::TangentialDistortionMode>(),
			UtilityFunctions::stringFromEnum(CalibrationParameters::Defaults::tangentialMode).value(),
			"Tangential distortion mode",
			std::nullopt,
			"Controls how tangential distortion parameters (p1, p2) are handled during calibration. In FIX mode, the tangential distortion coefficients are locked to the values provided by the user. This requires 'Use initial guess' to be enabled; otherwise OpenCV will ignore the user values and use its own estimates."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}

	// Use initial guess
	{
		std::shared_ptr<Parameter> aParam = ParameterBool::create(
			"use_initial_guess",
			CATEGORY,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			CalibrationParameters::Defaults::useInitialGuess,
			"Use initial guess",
			std::nullopt,
			"Enable this to use the user-provided camera parameters as the starting point for calibration. This is required when any of the following modes are enabled:\n\n"
			"- Principal Point Mode = FIX_AT_INITIAL\n"
			"- Focal Length Mode = FIX_BOTH\n"
			"- Tangential Distortion Mode = FIX\n\n"
			"If disabled, OpenCV will ignore user initial values and compute its own estimates."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}

	// Fix K1
	{
		std::shared_ptr<Parameter> aParam = ParameterBool::create(
			"fix_k1",
			CATEGORY,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			CalibrationParameters::Defaults::fixK1,
			"Fix K1",
			std::nullopt,
			"Prevents the first radial distortion coefficient (K1) from being optimized during calibration. The parameter remains fixed at its current value."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}

	// Fix K2
	{
		std::shared_ptr<Parameter> aParam = ParameterBool::create(
			"fix_k2",
			CATEGORY,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			CalibrationParameters::Defaults::fixK2,
			"Fix K2",
			std::nullopt,
			"Prevents the second radial distortion coefficient (K2) from being optimized during calibration. The parameter remains fixed at its current value."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}

	// Fix K3
	{
		std::shared_ptr<Parameter> aParam = ParameterBool::create(
			"fix_k3",
			CATEGORY,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			CalibrationParameters::Defaults::fixK3,
			"Fix K3",
			std::nullopt,
			"Prevents the third radial distortion coefficient (K3) from being optimized during calibration. The parameter remains fixed at its current value."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}

	// Fix K4
	{
		std::shared_ptr<Parameter> aParam = ParameterBool::create(
			"fix_k4",
			CATEGORY,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			CalibrationParameters::Defaults::fixK4,
			"Fix K4",
			std::nullopt,
			"Prevents the fourth radial distortion coefficient (K4) from being optimized during calibration. The parameter remains fixed at its current value."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}

	// Fix K5
	{
		std::shared_ptr<Parameter> aParam = ParameterBool::create(
			"fix_k5",
			CATEGORY,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			CalibrationParameters::Defaults::fixK5,
			"Fix K5",
			std::nullopt,
			"Prevents the fifth radial distortion coefficient (K5) from being optimized during calibration. The parameter remains fixed at its current value."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}

	// Fix K6
	{
		std::shared_ptr<Parameter> aParam = ParameterBool::create(
			"fix_k6",
			CATEGORY,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			CalibrationParameters::Defaults::fixK6,
			"Fix K6",
			std::nullopt,
			"Prevents the sixth radial distortion coefficient (K6) from being optimized during calibration. The parameter remains fixed at its current value."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}

	// Fix S1S2S3S4
	{
		std::shared_ptr<Parameter> aParam = ParameterBool::create(
			"fix_s1s2s3s4",
			CATEGORY,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			CalibrationParameters::Defaults::fixS1S2S3S4,
			"Fix S1S2S3S4",
			std::nullopt,
			"Prevents the thin prism distortion coefficients (S1, S2, S3, S4) from being optimized during calibration. These parameters model additional lens distortions present in some optical systems."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}

	// Fix TauXTauY
	{
		std::shared_ptr<Parameter> aParam = ParameterBool::create(
			"fix_tauxtauy",
			CATEGORY,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			CalibrationParameters::Defaults::fixTauXTauY,
			"Fix TauXTauY",
			std::nullopt,
			"Prevents the sensor tilt distortion coefficients (TauX and TauY) from being optimized during calibration. These parameters model small tilts between the image sensor and the optical axis."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}
}
