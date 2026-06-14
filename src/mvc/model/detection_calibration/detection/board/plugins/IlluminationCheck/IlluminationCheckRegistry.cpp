#include <parameter/numeric/ParameterNumeric.hpp>
#include "IlluminationCheckRegistry.hpp"
#include "IlluminationCheck.hpp"


const std::string IlluminationCheckRegistry::CATEGORY{ IlluminationCheck::ID };

IlluminationCheckRegistry::IlluminationCheckRegistry()
	: ParameterRegistry()
{
	registerAllParameters();
}

/////////////////////////////////////////////////////////////

void IlluminationCheckRegistry::registerAllParameters()
{
	// Mark dilation radius
	{
		std::shared_ptr<Parameter> aParam = ParameterDouble::create(
			"mark_dilation_radius",
			CATEGORY,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			IlluminationCheck::Defaults::markDilationRadius,
			1.0,
			20.0,
			0.1,
			"Mark dilation radius",
			"px",
			"Sets the radius (in pixels) used to dilate calibration mark regions before computing illumination metrics. A larger radius analyzes a broader neighborhood around each mark, making the measurement less sensitive to local noise but more influenced by global lighting gradients. Smaller values focus strictly on the mark regions and are useful when background areas are unreliable. Increase this value when lighting varies smoothly across the board; decrease it if nearby artifacts or reflections disturb the measurement."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}

	// Intensity deviation scale
	{
		std::shared_ptr<Parameter> aParam = ParameterDouble::create(
			"intensity_deviation_scale",
			CATEGORY,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			IlluminationCheck::Defaults::intensityDeviationScale,
			20.0,
			80.0,
			0.1,
			"Intensity deviation scale",
			std::nullopt,
			"Defines the intensity scale used to convert brightness deviations into score penalties. "
			"Both the deviation of individual marks from the board mean and the overall dispersion "
			"of mark intensities are divided by this value when computing the illumination score. "
			"Smaller values make the evaluation stricter, penalizing even small brightness "
			"differences between marks, while larger values make the plugin more tolerant "
			"of uneven illumination."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}

	// Score offset
	{
		std::shared_ptr<Parameter> aParam = ParameterDouble::create(
			"score_offset_factor",
			CATEGORY,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			IlluminationCheck::Defaults::scoreOffsetFactor,
			0.5,
			1.5,
			0.1,
			"Score offset",
			std::nullopt,
			"Applies an offset before the illumination score is scaled, effectively shifting how forgiving the evaluation is. Increasing this value makes the plugin slightly more tolerant of illumination variations, while decreasing it makes scoring stricter overall. This parameter is useful when many otherwise valid boards are rejected due to minor lighting inconsistencies."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}
}
