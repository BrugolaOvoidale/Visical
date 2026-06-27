#include "AprilTagParametersRegistry.hpp"
#include <UtilityFunctions.hpp>
#include <parameter/numeric/ParameterNumeric.hpp>
#include <parameter/enum/ParameterEnum.hpp>
#include <parameter/bool/ParameterBool.hpp>
#include "ArucoParametersRegistry.hpp"
#include "AprilTagParameters.hpp"


/////////////////////////////////////////////////////////////

const std::string AprilTagParametersRegistry::CATEGORY{ ArucoParametersRegistry::CATEGORY() + Parameter::CATEGORY_SEP + "april_tag" };

const std::string AprilTagParametersRegistry::CATEGORY_GEOMETRY{ CATEGORY + Parameter::CATEGORY_SEP + "geometry" };

const std::string AprilTagParametersRegistry::CATEGORY_DETECTION{ CATEGORY + Parameter::CATEGORY_SEP + "detection" };

/////////////////////////////////////////////////////////////

AprilTagParametersRegistry::AprilTagParametersRegistry()
	: ParameterRegistry()
{
	registerAllParameters();
}

/////////////////////////////////////////////////////////////

void AprilTagParametersRegistry::registerAllParameters()
{
	// Marker separation
	{
		std::shared_ptr<Parameter> aParam = ParameterDouble::create(
			"marker_separation",
			AprilTagParametersRegistry::CATEGORY_GEOMETRY,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			30.0,
			0.0,
			std::nullopt,
			std::nullopt,
			"Marker separation",
			"mm",
			"Specifies the physical separation between two markers, expressed in real-world units."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}

	// Dictionary
	{
		std::shared_ptr<Parameter> aParam = ParameterEnum::create(
			"dictionary",
			AprilTagParametersRegistry::CATEGORY_GEOMETRY,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			UtilityFunctions::enumValuesToIntMap<AprilTagDictionaries>(),
			cv::aruco::PredefinedDictionaryType::DICT_APRILTAG_36h11,
			"Dictionary",
			std::nullopt,
			"Each dictionary indicates the number of bits and the number of markers contained."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}

	// April Tag Quad Decimate
	{
		std::shared_ptr<Parameter> aParam = ParameterDouble::create(
			"april_tag_quad_decimate",
			AprilTagParametersRegistry::CATEGORY_DETECTION,
			AccessMode::READWRITE,
			VisibilityLevel::ADVANCED,
			AprilTagParameters::Detection::Defaults::arucoDetector.aprilTagQuadDecimate,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			"April tag quad decimate",
			std::nullopt,
			"Detection of quads can be done on a lower-resolution image, improving speed at a cost of pose accuracy and a slight decrease in detection rate. Decoding the binary payload is still completed on the full-resolution image."
		);

		registerParameter(aParam->getParameterView());
	}

	// April Tag Quad Sigma
	{
		std::shared_ptr<Parameter> aParam = ParameterDouble::create(
			"april_tag_quad_sigma",
			AprilTagParametersRegistry::CATEGORY_DETECTION,
			AccessMode::READWRITE,
			VisibilityLevel::ADVANCED,
			AprilTagParameters::Detection::Defaults::arucoDetector.aprilTagQuadSigma,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			"April tag quad sigma",
			std::nullopt,
			"What Gaussian blur should be applied to the segmented image (used for quad detection?)."
		);

		registerParameter(aParam->getParameterView());
	}

	// April Tag Min Cluster Pixels
	{
		std::shared_ptr<Parameter> aParam = ParameterInt::create(
			"april_tag_min_cluster_pixels",
			AprilTagParametersRegistry::CATEGORY_DETECTION,
			AccessMode::READWRITE,
			VisibilityLevel::ADVANCED,
			AprilTagParameters::Detection::Defaults::arucoDetector.aprilTagMinClusterPixels,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			"April tag min cluster pixels",
			std::nullopt,
			"Reject quads containing too few pixels (default 5)."
		);

		registerParameter(aParam->getParameterView());
	}

	// April Tag Max Nmaxima
	{
		std::shared_ptr<Parameter> aParam = ParameterInt::create(
			"april_tag_max_nmaxima",
			AprilTagParametersRegistry::CATEGORY_DETECTION,
			AccessMode::READWRITE,
			VisibilityLevel::ADVANCED,
			AprilTagParameters::Detection::Defaults::arucoDetector.aprilTagMaxNmaxima,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			"April tag max nmaxima",
			std::nullopt,
			"How many corner candidates to consider when segmenting a group of pixels into a quad."
		);

		registerParameter(aParam->getParameterView());
	}

	// April Tag Critical Rad
	{
		std::shared_ptr<Parameter> aParam = ParameterDouble::create(
			"april_tag_critical_rad",
			AprilTagParametersRegistry::CATEGORY_DETECTION,
			AccessMode::READWRITE,
			VisibilityLevel::ADVANCED,
			AprilTagParameters::Detection::Defaults::arucoDetector.aprilTagCriticalRad,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			"April tag critical rad",
			"rad",
			"Reject quads where pairs of edges have angles that are close to straight or close to 180 degrees. Zero means that no quads are rejected."
		);

		registerParameter(aParam->getParameterView());
	}

	// April Tag Max Line Fit Mse
	{
		std::shared_ptr<Parameter> aParam = ParameterDouble::create(
			"april_tag_max_line_fit_mse",
			AprilTagParametersRegistry::CATEGORY_DETECTION,
			AccessMode::READWRITE,
			VisibilityLevel::ADVANCED,
			AprilTagParameters::Detection::Defaults::arucoDetector.aprilTagMaxLineFitMse,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			"April tag max line fit mse",
			std::nullopt,
			"When fitting lines to the contours, what is the maximum mean squared error."
		);

		registerParameter(aParam->getParameterView());
	}

	// April Tag Min White Black Diff
	{
		std::shared_ptr<Parameter> aParam = ParameterInt::create(
			"april_tag_min_white_black_diff",
			AprilTagParametersRegistry::CATEGORY_DETECTION,
			AccessMode::READWRITE,
			VisibilityLevel::ADVANCED,
			AprilTagParameters::Detection::Defaults::arucoDetector.aprilTagMinWhiteBlackDiff,
			0,
			255,
			1,
			"April tag min white black diff",
			std::nullopt,
			"When we build our model of black & white pixels, we add an extra check that the white model must be (overall) brighter than the black model. How much brighter?"
		);

		registerParameter(aParam->getParameterView());
	}

	// April Tag Deglitch
	{
		std::shared_ptr<Parameter> aParam = ParameterInt::create(
			"april_tag_deglitch",
			AprilTagParametersRegistry::CATEGORY_DETECTION,
			AccessMode::READWRITE,
			VisibilityLevel::ADVANCED,
			AprilTagParameters::Detection::Defaults::arucoDetector.aprilTagDeglitch,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			"April tag deglitch",
			std::nullopt,
			"Should the thresholded image be deglitched? Only useful for very noisy images."
		);

		registerParameter(aParam->getParameterView());
	}

	// Corner Refinement Method
	{
		std::shared_ptr<Parameter> aParam = ParameterEnum::create(
			"corner_refinement_method",
			AprilTagParametersRegistry::CATEGORY_DETECTION,
			AccessMode::READWRITE,
			VisibilityLevel::ADVANCED,
			UtilityFunctions::enumValuesToIntMap<AprilTagCornerRefineMethod>(),
			AprilTagParameters::Detection::Defaults::arucoDetector.cornerRefinementMethod,
			"Corner refinement method",
			std::nullopt,
			"Method used for corner refinement."
		);

		registerParameter(aParam->getParameterView());
	}
}
