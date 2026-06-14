#include <parameter/numeric/ParameterNumeric.hpp>
#include "OverexposureCheckRegistry.hpp"
#include "OverexposureCheck.hpp"


const std::string OverexposureCheckRegistry::CATEGORY{ OverexposureCheck::ID };

OverexposureCheckRegistry::OverexposureCheckRegistry()
	: ParameterRegistry()
{
	registerAllParameters();
}

/////////////////////////////////////////////////////////////

void OverexposureCheckRegistry::registerAllParameters()
{
	// Overexposure lower bound
	{
		std::shared_ptr<Parameter> aParam = ParameterInt::create(
			"min_threshold",
			CATEGORY,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			OverexposureCheck::Defaults::minThreshold,
			0,
			255,
			1,
			"Overexposure lower bound",
			std::nullopt,
			"Defines the lower grayscale intensity value used to classify pixels as overexposed within the detected board region. Pixels with intensity values equal to or above this threshold are considered saturated or close to saturation. Lowering this value makes the check more sensitive, flagging mildly bright areas as overexposed, which can help detect early lighting issues but may penalize valid images. Increasing it restricts detection to only severely saturated pixels, making the evaluation more tolerant in bright environments but potentially missing problematic glare or reflections."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}

	// Overexposure upper bound
	{
		std::shared_ptr<Parameter> aParam = ParameterInt::create(
			"max_threshold",
			CATEGORY,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			OverexposureCheck::Defaults::maxThreshold,
			0,
			255,
			1,
			"Overexposure upper bound",
			std::nullopt,
			"Specifies the maximum grayscale intensity considered during overexposure detection. Pixels between the lower and this upper threshold are treated as overexposed. Typically this value remains at the maximum intensity, but lowering it allows excluding extreme sensor saturation or noise spikes. This parameter is rarely adjusted, but can help when cameras produce unstable peak intensities or clipped highlights."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}

	// Maximum allowed overexposed area
	{
		std::shared_ptr<Parameter> aParam = ParameterDouble::create(
			"max_area_ratio",
			CATEGORY,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			OverexposureCheck::Defaults::maxAreaRatio,
			0.02,
			0.40,
			0.01,
			"Maximum allowed overexposed area",
			std::nullopt,
			"Sets the maximum fraction of the detected board area that may be overexposed before image quality is penalized. If the ratio of overexposed pixels exceeds this value, the quality score rapidly drops to zero. Lower values enforce stricter lighting conditions and help avoid calibration failures due to glare or reflections. Higher values allow some saturation, which may be useful when working in uncontrolled lighting environments where avoiding reflections is difficult."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}
}
