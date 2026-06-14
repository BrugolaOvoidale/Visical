#include <parameter/numeric/ParameterNumeric.hpp>
#include "TiltCheckRegistry.hpp"
#include "TiltCheck.hpp"


const std::string TiltCheckRegistry::CATEGORY{ TiltCheck::ID };

TiltCheckRegistry::TiltCheckRegistry()
	: ParameterRegistry()
{
	registerAllParameters();
}

/////////////////////////////////////////////////////////////

void TiltCheckRegistry::registerAllParameters()
{
	// Maximum angle cap
	{
		std::shared_ptr<Parameter> aParam = ParameterDouble::create(
			"max_angle_cap",
			CATEGORY,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			TiltCheck::Defaults::maxAngleCap,
			45.0,
			90.0,
			0.1,
			"Maximum angle cap",
			"°",
			"Sets the upper limit (in degrees) for considering absolute slant or pan angles in the scoring; angles above this are capped to prevent extreme poses from overly influencing the score. Lower values make the plugin more sensitive to moderate tilts by narrowing the effective range, while higher values allow greater tolerance for steep angles, useful for wide-angle cameras or unconventional board orientations."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}

	// Angle scaling target
	{
		std::shared_ptr<Parameter> aParam = ParameterDouble::create(
			"angle_scaling_target",
			CATEGORY,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			TiltCheck::Defaults::angleScalingTarget,
			45.0,
			180.0,
			0.1,
			"Angle scaling target",
			"°",
			"Defines the target value (in degrees) to which capped angles are scaled before applying the cosine-based scoring function. This controls the periodicity and sensitivity of the score calculation; smaller targets compress the angle range for stricter scoring on small tilts, while larger ones spread it out, making the plugin more forgiving for varied poses in multi-board sequences."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}

	// Score exponent
	{
		std::shared_ptr<Parameter> aParam = ParameterDouble::create(
			"score_exponent",
			CATEGORY,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			TiltCheck::Defaults::scoreExponent,
			1.0,
			4.0,
			0.1,
			"Score exponent",
			std::nullopt,
			"Determines the power to which the cosine of the scaled angle is raised in the scoring formula (abs(pow(cos(v * 2 * pi / 180), exponent))). Higher values sharpen the scoring curve, penalizing angles farther from ideal (e.g., 0 degrees) more aggressively, which can emphasize optimal poses. Lower values soften it, allowing broader acceptance of tilted boards for robust calibration in challenging environments."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}

	// Score adjustment multiplier
	{
		std::shared_ptr<Parameter> aParam = ParameterDouble::create(
			"score_adjustment_multiplier",
			CATEGORY,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			TiltCheck::Defaults::scoreAdjustmentMultiplier,
			1.0,
			1.5,
			0.1,
			"Score adjustment multiplier",
			std::nullopt,
			"A factor applied to the raw cosine-based score before clamping to [0, 1], providing a slight boost to near-perfect scores. Setting it to 1.0 disables adjustment for purely mathematical scoring, while higher values inflate scores modestly, helping to differentiate good poses in high-quality sequences without over-rewarding mediocre ones."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}

	// Minimum images for scoring
	{
		std::shared_ptr<Parameter> aParam = ParameterInt::create(
			"min_image_count",
			CATEGORY,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			TiltCheck::Defaults::minImages,
			1,
			10,
			1,
			"Minimum images for scoring",
			std::nullopt,
			"Specifies the number of top-sorted scores (per slant and pan) to average for the final quality; if fewer boards are available, it uses all. Lower values focus on the best poses, potentially yielding higher scores for sequences with a few excellent boards. Higher values incorporate more data, providing a more representative average but risking lower scores if there are outliers, ideal for ensuring consistent tilt coverage."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}

	// Slant weight
	{
		std::shared_ptr<Parameter> aParam = ParameterDouble::create(
			"slant_weight",
			CATEGORY,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			TiltCheck::Defaults::slantWeight,
			0.0,
			1.0,
			0.1,
			"Slant weight",
			std::nullopt,
			"Sets the relative importance of the slant (pitch/tilt) score in the final quality average. Increase this to prioritize vertical tilts (e.g., for overhead or ground-facing cameras), reducing the influence of horizontal pans. Use in conjunction with Pan weight to customize emphasis based on application-specific distortion concerns."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}

	// Pan weight
	{
		std::shared_ptr<Parameter> aParam = ParameterDouble::create(
			"pan_weight",
			CATEGORY,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			TiltCheck::Defaults::panWeight,
			0.0,
			1.0,
			0.1,
			"Pan weight",
			std::nullopt,
			"Sets the relative importance of the pan (yaw) score in the final quality average. Increase this to emphasize horizontal rotations (e.g., for side-view or panning scenarios), downplaying slant impacts. Adjust alongside Slant weight to tailor the plugin for specific camera calibration needs, such as fisheye lenses where one axis dominates distortion."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}
}
