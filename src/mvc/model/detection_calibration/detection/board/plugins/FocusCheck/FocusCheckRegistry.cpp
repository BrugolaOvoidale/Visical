#include "FocusCheckRegistry.hpp"
#include <parameter/numeric/ParameterNumeric.hpp>
#include "FocusCheck.hpp"


const std::string FocusCheckRegistry::CATEGORY{ FocusCheck::ID };

FocusCheckRegistry::FocusCheckRegistry()
	: ParameterRegistry()
{
	registerAllParameters();
}

/////////////////////////////////////////////////////////////

void FocusCheckRegistry::registerAllParameters()
{
	// Minimum marks required
	{
		std::shared_ptr<Parameter> aParam = ParameterInt::create(
			"min_marks_count",
			CATEGORY,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			FocusCheck::Defaults::minMarksCount,
			1,
			std::nullopt,
			1,
			"Minimum marks required",
			std::nullopt,
			"Sets the minimum number of detected calibration marks required to evaluate focus. Boards with fewer marks than this threshold are skipped or marked as failed. Increasing this value enforces stricter quality control, ensuring enough data for reliable focus computation. Lowering it allows evaluation of partially visible or small boards, useful in constrained environments."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}
	
	// Mark dilation radius
	{
		std::shared_ptr<Parameter> aParam = ParameterDouble::create(
			"mark_dilation_radius",
			CATEGORY,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			FocusCheck::Defaults::markDilationRadius,
			1.0,
			20.0,
			0.1,
			"Mark dilation radius",
			"px",
			"Sets the radius (in pixels) used to dilate calibration mark regions before computing focus metrics. Larger values smooth out local variations and capture more surrounding pixels, reducing sensitivity to noise, while smaller values concentrate the evaluation strictly on the mark area, making the plugin more sensitive to fine focus differences."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}

	// Sigma
	{
		std::shared_ptr<Parameter> aParam = ParameterDouble::create(
			"sigma",
			CATEGORY,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			FocusCheck::Defaults::sigma,
			0.1,
			3.0,
			0.1,
			"Sigma",
			std::nullopt,
			"Controls the sigma value for the Gaussian derivative used to compute edge gradients. Lower values detect finer edges, increasing sensitivity to subtle focus differences, while higher values smooth out noise and emphasize stronger edges, useful for images with higher noise or less-defined marks."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}

	// Gradient scaling factor
	{
		std::shared_ptr<Parameter> aParam = ParameterDouble::create(
			"gradient_scaling_factor",
			CATEGORY,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			FocusCheck::Defaults::gradientScalingFactor,
			1.0,
			10.0,
			0.1,
			"Gradient scaling factor",
			std::nullopt,
			"Multiplies the normalized gradient values before selecting the representative focus score. Increasing this factor amplifies differences between well-focused and slightly defocused boards, making the score more responsive to sharpness variations. Lower values produce more conservative scoring."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}

	// Gradient percentile divisor
	{
		std::shared_ptr<Parameter> aParam = ParameterDouble::create(
			"gradient_percentile_divisor",
			CATEGORY,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			FocusCheck::Defaults::gradientPercentileDivisor,
			5.0,
			50.0,
			0.1,
			"Gradient percentile divisor",
			std::nullopt,
			"Controls which percentile of gradient responses is selected to compute the focus score. Smaller values select only the very strongest edges, making the evaluation stricter and more sensitive to blur. Larger values include more moderate edges, producing more stable and tolerant scoring when marks are noisy or partially degraded. Lower values are suitable for high-quality images, while higher values help when images contain noise or imperfect detections."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}

	// Minimum score offset
	{
		std::shared_ptr<Parameter> aParam = ParameterDouble::create(
			"min_score_offset",
			CATEGORY,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			FocusCheck::Defaults::minScoreOffset,
			0.0,
			0.5,
			0.01,
			"Minimum score offset",
			std::nullopt,
			"Subtracts a baseline from the computed focus metric to define the minimum acceptable score. Higher values make the plugin stricter by requiring higher contrast and edge sharpness to reach a good score, while lower values allow boards with weaker focus to pass with higher quality percentages."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}
}
