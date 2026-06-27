#include "ChessboardParametersRegistry.hpp"
#include <UtilityFunctions.hpp>
#include <parameter/numeric/ParameterNumeric.hpp>
#include <parameter/enum/ParameterEnum.hpp>
#include <parameter/bool/ParameterBool.hpp>
#include "PatternParametersRegistry.hpp"
#include "ChessboardParameters.hpp"


/////////////////////////////////////////////////////////////

const std::string ChessboardParametersRegistry::CATEGORY{ PatternParametersRegistry::CATEGORY() + Parameter::CATEGORY_SEP + "chessboard" };

const std::string ChessboardParametersRegistry::CATEGORY_GEOMETRY{ CATEGORY + Parameter::CATEGORY_SEP + "geometry" };

const std::string ChessboardParametersRegistry::CATEGORY_DETECTION{ CATEGORY + Parameter::CATEGORY_SEP + "detection" };

const std::string ChessboardParametersRegistry::CATEGORY_REFINE{ CATEGORY + Parameter::CATEGORY_SEP + "refine" };

/////////////////////////////////////////////////////////////

ChessboardParametersRegistry::ChessboardParametersRegistry()
	: ParameterRegistry()
{
	registerAllParameters();
}

/////////////////////////////////////////////////////////////

void ChessboardParametersRegistry::registerAllParameters()
{
	// Square size
	{
		std::shared_ptr<Parameter> aParam = ParameterDouble::create(
			"square_size",
			ChessboardParametersRegistry::CATEGORY_GEOMETRY,
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

	// Search accuracy
	{
		std::shared_ptr<Parameter> aParam = ParameterEnum::create(
			"search_accuracy",
			ChessboardParametersRegistry::CATEGORY_DETECTION,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			UtilityFunctions::enumValuesToIntMap<ChessboardParameters::Detection::SearchAccuracy>(),
			UtilityFunctions::stringFromEnum(ChessboardParameters::Detection::Defaults::searchAccuracy).value(),
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
			ChessboardParametersRegistry::CATEGORY_DETECTION,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			ChessboardParameters::Detection::Defaults::allowLargerBoards,
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
			ChessboardParametersRegistry::CATEGORY_DETECTION,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			ChessboardParameters::Detection::Defaults::subpixelAccuracy,
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
			ChessboardParametersRegistry::CATEGORY_DETECTION,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			ChessboardParameters::Detection::Defaults::filterQuads,
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
			ChessboardParametersRegistry::CATEGORY_DETECTION,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			ChessboardParameters::Detection::Defaults::normalizeImage,
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
			ChessboardParametersRegistry::CATEGORY_DETECTION,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			ChessboardParameters::Detection::Defaults::adaptiveThreshold,
			"Adaptive threshold",
			std::nullopt,
			"Uses adaptive thresholding during the preprocessing stage of chessboard detection. This allows the algorithm to better handle varying lighting conditions across the image by computing local threshold values instead of a single global threshold."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}

	// Termination criteria type
	{
		std::shared_ptr<Parameter> aParam = ParameterEnum::create(
			"term_criteria_type",
			ChessboardParametersRegistry::CATEGORY_REFINE,
			AccessMode::READWRITE,
			VisibilityLevel::ADVANCED,
			UtilityFunctions::enumValuesToIntMap<ChessboardParameters::Refine::TermCriteriaType>(),
			UtilityFunctions::stringFromEnum(ChessboardParameters::Refine::Defaults::type).value(),
			"Termination criteria type",
			std::nullopt,
			"Defines the termination criteria for iterative sub-pixel refinement algorithms. COUNT uses only the maximum number of iterations, EPSILON uses only the desired accuracy, and BOTH uses both criteria (whichever comes first)."
		);

		registerParameter(aParam->getParameterView());
	}

	// Maximum iterations
	{
		std::shared_ptr<Parameter> aParam = ParameterInt::create(
			"max_iterations",
			ChessboardParametersRegistry::CATEGORY_REFINE,
			AccessMode::READWRITE,
			VisibilityLevel::ADVANCED,
			ChessboardParameters::Refine::Defaults::maxCount,
			1,
			1000,
			std::nullopt,
			"Maximum iterations",
			std::nullopt,
			"The maximum number of iterations for the sub-pixel corner refinement algorithm. Higher values allow more refinement but increase computation time."
		);

		registerParameter(aParam->getParameterView());
	}

	// Epsilon accuracy
	{
		std::shared_ptr<Parameter> aParam = ParameterDouble::create(
			"epsilon",
			ChessboardParametersRegistry::CATEGORY_REFINE,
			AccessMode::READWRITE,
			VisibilityLevel::ADVANCED,
			ChessboardParameters::Refine::Defaults::epsilon,
			0.0,
			1.0,
			0.0001,
			"Epsilon accuracy",
			std::nullopt,
			"The desired accuracy or change in parameters at which the iterative sub-pixel refinement algorithm stops. Smaller values give more precise results but may require more iterations."
		);

		registerParameter(aParam->getParameterView());
	}

	// Search window width
	{
		std::shared_ptr<Parameter> aParam = ParameterInt::create(
			"window_width",
			ChessboardParametersRegistry::CATEGORY_REFINE,
			AccessMode::READWRITE,
			VisibilityLevel::ADVANCED,
			ChessboardParameters::Refine::Defaults::windowWidth,
			1,
			51,
			std::nullopt,
			"Search window width",
			std::nullopt,
			"Half of the side length of the search window for sub-pixel refinement. The actual window size is (2*width+1) x (2*height+1). Larger windows can handle more distortion but may be affected by nearby corners. Must be an odd number."
		);

		registerParameter(aParam->getParameterView());
	}

	// Search window height
	{
		std::shared_ptr<Parameter> aParam = ParameterInt::create(
			"window_height",
			ChessboardParametersRegistry::CATEGORY_REFINE,
			AccessMode::READWRITE,
			VisibilityLevel::ADVANCED,
			ChessboardParameters::Refine::Defaults::windowHeight,
			1,
			51,
			std::nullopt,
			"Search window height",
			std::nullopt,
			"Half of the side length of the search window for sub-pixel refinement. The actual window size is (2*width+1) x (2*height+1). Larger windows can handle more distortion but may be affected by nearby corners. Must be an odd number."
		);

		registerParameter(aParam->getParameterView());
	}

	// Zero zone width
	{
		std::shared_ptr<Parameter> aParam = ParameterInt::create(
			"zero_zone_width",
			ChessboardParametersRegistry::CATEGORY_REFINE,
			AccessMode::READWRITE,
			VisibilityLevel::ADVANCED,
			ChessboardParameters::Refine::Defaults::zeroZoneWidth,
			-1,
			25,
			std::nullopt,
			"Zero zone width",
			std::nullopt,
			"Half of the side length of the zero zone within the search window. The zero zone is the region around the center where the algorithm ignores the gradient information. A value of -1 means no zero zone. Typically used to avoid singularities at the corner center."
		);

		registerParameter(aParam->getParameterView());
	}

	// Zero zone height
	{
		std::shared_ptr<Parameter> aParam = ParameterInt::create(
			"zero_zone_height",
			ChessboardParametersRegistry::CATEGORY_REFINE,
			AccessMode::READWRITE,
			VisibilityLevel::ADVANCED,
			ChessboardParameters::Refine::Defaults::zeroZoneHeight,
			-1,
			25,
			std::nullopt,
			"Zero zone height",
			std::nullopt,
			"Half of the side length of the zero zone within the search window. The zero zone is the region around the center where the algorithm ignores the gradient information. A value of -1 means no zero zone. Typically used to avoid singularities at the corner center."
		);

		registerParameter(aParam->getParameterView());
	}
}
