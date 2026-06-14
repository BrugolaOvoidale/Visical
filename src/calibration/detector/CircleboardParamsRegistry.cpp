#include "CircleboardParamsRegistry.hpp"
#include <parameter/numeric/ParameterNumeric.hpp>
#include <parameter/bool/ParameterBool.hpp>
#include "PatternParamsRegistry.hpp"
#include "CircleboardParams.hpp"


/////////////////////////////////////////////////////////////

const std::string CircleboardParamsRegistry::CATEGORY{ PatternParamsRegistry::CATEGORY() + Parameter::CATEGORY_SEP + "circleboard" };

/////////////////////////////////////////////////////////////

CircleboardParamsRegistry::CircleboardParamsRegistry()
	: ParameterRegistry()
{
	registerAllParameters();
}

/////////////////////////////////////////////////////////////

void CircleboardParamsRegistry::registerAllParameters()
{
	// Mark diameter
	{
		std::shared_ptr<Parameter> aParam = ParameterDouble::create(
			"mark_diameter",
			CircleboardParamsRegistry::CATEGORY,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			30.0,
			0.0,
			std::nullopt,
			std::nullopt,
			"Mark diameter",
			"mm",
			"Specifies the physical diameter of each circular mark on the calibration board, expressed in real-world units. This value is used to define the geometric layout of the pattern and contributes to computing the 3D coordinates of the detected points during calibration."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}

	// Center distance
	{
		std::shared_ptr<Parameter> aParam = ParameterDouble::create(
			"center_distance",
			CircleboardParamsRegistry::CATEGORY,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			30.0,
			0.0,
			std::nullopt,
			std::nullopt,
			"Center distance",
			"mm",
			"Specifies the physical distance between the centers of adjacent circular marks on the board. This value defines the spacing of the pattern and must match the actual board geometry to ensure correct point layout during calibration."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}

	// Use clustering
	{
		std::shared_ptr<Parameter> aParam = ParameterBool::create(
			"use_clustering",
			CircleboardParamsRegistry::CATEGORY,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			CircleboardParams::Defaults::useClustering,
			"Use clustering",
			std::nullopt,
			"Enables clustering of detected circular candidates during pattern detection. Clustering groups nearby detections to improve robustness when identifying the grid structure, particularly in noisy images or when false circle detections are present."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}
}
