#include "PatternParamsRegistry.hpp"
#include <UtilityFunctions.hpp>
#include <parameter/enum/ParameterEnum.hpp>
#include <parameter/numeric/ParameterNumeric.hpp>
#include "PatternParams.hpp"


const std::string& PatternParamsRegistry::CATEGORY()
{
	static const std::string category{ "board_detection" };

	return category;
}

PatternParamsRegistry::PatternParamsRegistry()
	: ParameterRegistry()
{
	registerAllParameters();
}

/////////////////////////////////////////////////////////////

void PatternParamsRegistry::registerAllParameters()
{
	// Board pattern
	{
		std::shared_ptr<Parameter> aParam = ParameterEnum::create(
			"pattern_type",
			CATEGORY(),
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			UtilityFunctions::enumValuesToIntMap<BoardPattern>(),
			UtilityFunctions::stringFromEnum<BoardPattern>(BoardPattern::CHESSBOARD).value(),
			"Board pattern",
			std::nullopt,
			"Specifies the type of calibration pattern used on the board. The selected pattern determines which detection algorithm is used and how feature points are extracted from the image."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}

	// Pattern width
	{
		std::shared_ptr<Parameter> aParam = ParameterInt::create(
			"pattern_cols",
			CATEGORY(),
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			8,
			1,
			std::nullopt,
			std::nullopt,
			"Pattern width",
			"",
			"Number of pattern marks along the horizontal direction of the board. For example, in a chessboard this corresponds to the number of inner corners detected per row. This value must match the physical calibration board for correct detection and calibration."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}

	// Pattern height
	{
		std::shared_ptr<Parameter> aParam = ParameterInt::create(
			"pattern_rows",
			CATEGORY(),
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			5,
			1,
			std::nullopt,
			std::nullopt,
			"Pattern height",
			"",
			"Number of pattern marks along the vertical direction of the board. For example, in a chessboard this corresponds to the number of inner corners detected per column. This value must match the physical calibration board for correct detection and calibration."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}
}
