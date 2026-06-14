#include <parameter/bool/ParameterBool.hpp>
#include <parameter/numeric/ParameterNumeric.hpp>
#include "DetectionModelRegistry.hpp"


DetectionModelRegistry::DetectionModelRegistry()
	: ParameterRegistry()
{
	registerAllParameters();
}

/////////////////////////////////////////////////////////////

void DetectionModelRegistry::registerAllParameters()
{
	// Mirror X
	{
		std::shared_ptr<Parameter> aParam = ParameterBool::create(
			"mirror_x",
			CATEGORY_PRE_PROC,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			false,
			"Mirror X",
			std::nullopt,
			"Mirror X"
		);

		registerParameter(aParam->getParameterView());
	}

	// Mirror Y
	{
		std::shared_ptr<Parameter> aParam = ParameterBool::create(
			"mirror_y",
			CATEGORY_PRE_PROC,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			false,
			"Mirror Y",
			std::nullopt,
			"Mirror Y"
		);

		registerParameter(aParam->getParameterView());
	}

	// Rotate image
	{
		std::shared_ptr<Parameter> aParam = ParameterInt::create(
			"rotate",
			CATEGORY_PRE_PROC,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			0,
			-90,
			90,
			90,
			"Rotate image",
			"°",
			"Rotate image"
		);

		registerParameter(aParam->getParameterView());
	}
}