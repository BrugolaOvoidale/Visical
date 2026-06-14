#include "ChessboardParamsRegistry.hpp"
#include <UtilityFunctions.hpp>
#include <parameter/numeric/ParameterNumeric.hpp>
#include <parameter/enum/ParameterEnum.hpp>
#include <parameter/bool/ParameterBool.hpp>
#include "PatternParamsRegistry.hpp"
#include "ChessboardParams.hpp"


/////////////////////////////////////////////////////////////

const std::string ChessboardParamsRegistry::CATEGORY{ PatternParamsRegistry::CATEGORY() + Parameter::CATEGORY_SEP + "chessboard" };

/////////////////////////////////////////////////////////////

ChessboardParamsRegistry::ChessboardParamsRegistry()
	: ParameterRegistry()
{
	registerAllParameters();
}

/////////////////////////////////////////////////////////////

void ChessboardParamsRegistry::registerAllParameters()
{
	// Square size
	{
		std::shared_ptr<Parameter> aParam = ParameterDouble::create(
			"square_size",
			ChessboardParamsRegistry::CATEGORY,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			30.0,
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

	// Search accuracy
	{
		std::shared_ptr<Parameter> aParam = ParameterEnum::create(
			"search_accuracy",
			ChessboardParamsRegistry::CATEGORY,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			UtilityFunctions::enumValuesToIntMap<ChessboardParams::SearchAccuracy>(),
			UtilityFunctions::stringFromEnum(ChessboardParams::Defaults::searchAccuracy).value(),
			"Search accuracy",
			std::nullopt,
			"Controls the accuracy level used by the chessboard detection algorithm. Higher accuracy settings perform more exhaustive searches and additional refinement steps, improving robustness and corner localization at the cost of increased computation time."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}

	// Allow larger boards
	{
		std::shared_ptr<Parameter> aParam = ParameterBool::create(
			"allow_larger_boards",
			ChessboardParamsRegistry::CATEGORY,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			ChessboardParams::Defaults::allowLargerBoards,
			"Allow larger boards",
			std::nullopt,
			"Allows the detector to accept chessboard patterns that are larger than the specified pattern dimensions. When enabled, the algorithm searches for valid sub-patterns matching the configured size within a larger detected board."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}

	// Subpixel accuracy
	{
		std::shared_ptr<Parameter> aParam = ParameterBool::create(
			"subpixel_accuracy",
			ChessboardParamsRegistry::CATEGORY,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			ChessboardParams::Defaults::subpixelAccuracy,
			"Subpixel accuracy",
			std::nullopt,
			"Enables subpixel refinement of detected chessboard corners. When enabled, corner positions are refined to subpixel precision, improving the accuracy of the extracted feature points and resulting camera calibration."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}

	// Filter quads
	{
		std::shared_ptr<Parameter> aParam = ParameterBool::create(
			"filter_quads",
			ChessboardParamsRegistry::CATEGORY,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			ChessboardParams::Defaults::filterQuads,
			"Filter quads",
			std::nullopt,
			"Enables additional filtering of quadrilateral candidates during chessboard detection. This step removes unlikely square candidates based on geometric constraints, improving robustness in noisy images but potentially rejecting difficult detections."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}

	// Normalize image
	{
		std::shared_ptr<Parameter> aParam = ParameterBool::create(
			"normalize_image",
			ChessboardParamsRegistry::CATEGORY,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			ChessboardParams::Defaults::normalizeImage,
			"Normalize image",
			std::nullopt,
			"Applies image normalization before detection to improve contrast across the image. This can help detect chessboard patterns in images with uneven illumination or low contrast."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}

	// Adaptive threshold
	{
		std::shared_ptr<Parameter> aParam = ParameterBool::create(
			"adaptive_threshold",
			ChessboardParamsRegistry::CATEGORY,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			ChessboardParams::Defaults::adaptiveThreshold,
			"Adaptive threshold",
			std::nullopt,
			"Uses adaptive thresholding during the preprocessing stage of chessboard detection. This allows the algorithm to better handle varying lighting conditions across the image by computing local threshold values instead of a single global threshold."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}
}
