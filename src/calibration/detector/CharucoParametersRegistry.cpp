#include "CharucoParametersRegistry.hpp"
#include <UtilityFunctions.hpp>
#include <parameter/numeric/ParameterNumeric.hpp>
#include <parameter/enum/ParameterEnum.hpp>
#include <parameter/bool/ParameterBool.hpp>
#include "ArucoParametersRegistry.hpp"
#include "CharucoParameters.hpp"


/////////////////////////////////////////////////////////////

const std::string CharucoParametersRegistry::CATEGORY{ ArucoParametersRegistry::CATEGORY() + Parameter::CATEGORY_SEP + "charuco" };

const std::string CharucoParametersRegistry::CATEGORY_GEOMETRY{ CATEGORY + Parameter::CATEGORY_SEP + "geometry" };

const std::string CharucoParametersRegistry::CATEGORY_DETECTION{ CATEGORY + Parameter::CATEGORY_SEP + "detection" };

/////////////////////////////////////////////////////////////

CharucoParametersRegistry::CharucoParametersRegistry()
	: ParameterRegistry()
{
	registerAllParameters();
}

/////////////////////////////////////////////////////////////

void CharucoParametersRegistry::registerAllParameters()
{
	// Square size
	{
		std::shared_ptr<Parameter> aParam = ParameterDouble::create(
			"square_size",
			CharucoParametersRegistry::CATEGORY_GEOMETRY,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			10.0,
			0.0,
			std::nullopt,
			std::nullopt,
			"Square size",
			"mm",
			"Specifies the physical size of one square on the chessboard pattern, expressed in real-world units. This value is used to compute the 3D coordinates of the calibration points and therefore determines the scale of the resulting camera calibration. The unit must match the one used for calibration outputs."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}

	// Dictionary
	{
		std::shared_ptr<Parameter> aParam = ParameterEnum::create(
			"dictionary",
			CharucoParametersRegistry::CATEGORY_GEOMETRY,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			UtilityFunctions::enumValuesToIntMap<CharucoDictionaries>(),
			cv::aruco::PredefinedDictionaryType::DICT_4X4_50,
			"Dictionary",
			std::nullopt,
			"Each dictionary indicates the number of bits and the number of markers contained."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}

	// Minimum markers
	{
		std::shared_ptr<Parameter> aParam = ParameterInt::create(
			"min_markers",
			CharucoParametersRegistry::CATEGORY_DETECTION,
			AccessMode::READWRITE,
			VisibilityLevel::INTERMEDIATE,
			CharucoParameters::Detection::Defaults::charuco.minMarkers,
			1,
			std::nullopt,
			std::nullopt,
			"Minimum markers",
			std::nullopt,
			"Number of adjacent markers that must be detected to return a charuco corner."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}

	// Try refine markers
	{
		std::shared_ptr<Parameter> aParam = ParameterBool::create(
			"try_refine_markers",
			CharucoParametersRegistry::CATEGORY_DETECTION,
			AccessMode::READWRITE,
			VisibilityLevel::INTERMEDIATE,
			CharucoParameters::Detection::Defaults::charuco.tryRefineMarkers,
			"Try refine markers",
			std::nullopt,
			"Try to use refine board."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}

	// Corner Refinement Method
	{
		std::shared_ptr<Parameter> aParam = ParameterEnum::create(
			"corner_refinement_method",
			CharucoParametersRegistry::CATEGORY_DETECTION,
			AccessMode::READWRITE,
			VisibilityLevel::ADVANCED,
			UtilityFunctions::enumValuesToIntMap<CharucoCornerRefineMethod>(),
			CharucoParameters::Detection::Defaults::arucoDetector.cornerRefinementMethod,
			"Corner refinement method",
			std::nullopt,
			"Method used for corner refinement."
		);

		registerParameter(aParam->getParameterView());
	}
}
