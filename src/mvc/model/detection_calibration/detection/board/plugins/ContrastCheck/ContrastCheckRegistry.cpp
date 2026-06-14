#include "ContrastCheckRegistry.hpp"
#include <parameter/numeric/ParameterNumeric.hpp>
#include "ContrastCheck.hpp"


const std::string ContrastCheckRegistry::CATEGORY{ ContrastCheck::ID };

ContrastCheckRegistry::ContrastCheckRegistry()
	: ParameterRegistry()
{
	registerAllParameters();
}

/////////////////////////////////////////////////////////////

void ContrastCheckRegistry::registerAllParameters()
{
	// Mark dilation radius
	{
		std::shared_ptr<Parameter> aParam = ParameterDouble::create(
			"mark_dilation_radius",
			CATEGORY,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			ContrastCheck::Defaults::markDilationRadius,
			1.0,
			20.0,
			0.1,
			"Mark dilation radius",
			"px",
			"Controls the radius (in pixels) used to dilate the detected marks when computing contrast. A larger radius includes surrounding regions in the analysis, smoothing small local variations and emphasizing overall mark visibility, while a smaller radius focuses strictly on the mark contours, making the evaluation more sensitive to small, sharp features. Adjusting this can help account for camera resolution and mark size."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}

	// Minimum detectable contrast
	{
		std::shared_ptr<Parameter> aParam = ParameterInt::create(
			"min_detectable_contrast",
			CATEGORY,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			ContrastCheck::Defaults::minDetectableContrast,
			0,
			255,
			1,
			"Minimum detectable contrast",
			std::nullopt,
			"Sets the lower bound of gray-level difference considered for contrast evaluation. Values below this threshold are treated as insufficient contrast and will produce a low score. Lowering this value makes the plugin more tolerant of dim or low-contrast boards, while increasing it makes the plugin stricter, requiring more pronounced gray-level differences to achieve a high quality score."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}


	// Sufficient contrast threshold
	{
		std::shared_ptr<Parameter> aParam = ParameterInt::create(
			"sufficient_contrast_threshold",
			CATEGORY,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			ContrastCheck::Defaults::sufficientContrastThr,
			0,
			255,
			1,
			"Sufficient contrast threshold",
			std::nullopt,
			"Defines the contrast level at which a board is considered fully satisfactory. Contrasts at or above this threshold give a maximum quality score. Lowering this value allows boards with moderate contrast to be rated highly, whereas increasing it raises the standard, ensuring only very high-contrast boards reach full score, useful for demanding lighting conditions."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}
}
