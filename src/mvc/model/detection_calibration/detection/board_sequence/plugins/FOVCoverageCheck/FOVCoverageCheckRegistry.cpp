#include "FOVCoverageCheckRegistry.hpp"
#include <parameter/numeric/ParameterNumeric.hpp>
#include "FOVCoverageCheck.hpp"


const std::string FOVCoverageCheckRegistry::CATEGORY{ FOVCoverageCheck::ID };

FOVCoverageCheckRegistry::FOVCoverageCheckRegistry()
	: ParameterRegistry()
{
	registerAllParameters();
}

/////////////////////////////////////////////////////////////

void FOVCoverageCheckRegistry::registerAllParameters()
{
	// Border exclusion margin
	{
		std::shared_ptr<Parameter> aParam = ParameterDouble::create(
			"border_exclusion_ratio",
			CATEGORY,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			FOVCoverageCheck::Defaults::borderExclusionRatio,
			0.02,
			0.2,
			0.001,
			"Border exclusion margin",
			std::nullopt,
			"Controls the proportional size of the border area excluded from the maximum distance calculation to ignore unreliable edge regions of the image. A lower value results in a smaller excluded border, allowing more of the image to contribute to the coverage assessment, which may be useful for smaller images or when edge coverage is important. A higher value increases the border, focusing the evaluation on the central area and potentially improving accuracy in larger or distorted images."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}

	// Coverage sensitivity
	{
		std::shared_ptr<Parameter> aParam = ParameterDouble::create(
			"coverage_normalization_factor",
			CATEGORY,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			FOVCoverageCheck::Defaults::coverageNormalizationFactor,
			1.0,
			5.0,
			0.1,
			"Coverage sensitivity",
			std::nullopt,
			"Scales the ratio of the maximum distance (from points to uncovered areas) to the image diagonal in the coverage ratio calculation. Lower values make the plugin less sensitive to gaps in point coverage, potentially yielding higher quality scores for sparsely distributed boards. Higher values increase sensitivity, penalizing larger uncovered areas more strictly, which can help enforce better overall FOV coverage in calibration sequences."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}

	// Minimum acceptable coverage
	{
		std::shared_ptr<Parameter> aParam = ParameterDouble::create(
			"min_coverage_threshold",
			CATEGORY,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			FOVCoverageCheck::Defaults::minCoverageThreshold,
			0.1,
			0.5,
			0.1,
			"Minimum acceptable coverage",
			std::nullopt,
			"Sets the lower bound for the inverted coverage ratio (1 - Ratio) below which the quality score is clamped to 0%. This controls the point at which coverage is deemed completely inadequate; increasing it raises the bar for acceptable coverage, making the plugin stricter for low-quality boards, while decreasing it allows more leniency for marginal cases."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}

	// Maximum acceptable coverage
	{
		std::shared_ptr<Parameter> aParam = ParameterDouble::create(
			"max_coverage_threshold",
			CATEGORY,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			FOVCoverageCheck::Defaults::maxCoverageThreshold,
			0.6,
			1.0,
			0.1,
			"Maximum Acceptable Coverage",
			std::nullopt,
			"Sets the upper bound for the inverted coverage ratio (1 - Ratio) above which the quality score is clamped to 100%. This defines when coverage is considered optimal; lowering it makes it easier to achieve full scores for good-but-not-perfect coverage, while raising it requires near-perfect point distribution for high scores, useful for high-precision calibration needs."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}
}
