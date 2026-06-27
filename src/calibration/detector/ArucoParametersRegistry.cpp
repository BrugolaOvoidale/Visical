#include "ArucoParametersRegistry.hpp"
#include <UtilityFunctions.hpp>
#include <parameter/numeric/ParameterNumeric.hpp>
#include <parameter/enum/ParameterEnum.hpp>
#include <parameter/bool/ParameterBool.hpp>
#include "PatternParametersRegistry.hpp"
#include "ArucoParameters.hpp"


/////////////////////////////////////////////////////////////

const std::string& ArucoParametersRegistry::CATEGORY()
{
	static const std::string category{ PatternParametersRegistry::CATEGORY() + Parameter::CATEGORY_SEP + "aruco" };

	return category;
};

const std::string& ArucoParametersRegistry::CATEGORY_GEOMETRY()
{
	static const std::string category{ CATEGORY() + Parameter::CATEGORY_SEP + "geometry"};

	return category;
};

const std::string& ArucoParametersRegistry::CATEGORY_DETECTION()
{
	static const std::string category{ CATEGORY() + Parameter::CATEGORY_SEP + "detection" };
	
	return category;
};

const std::string& ArucoParametersRegistry::CATEGORY_REFINE()
{
	static const std::string category{ CATEGORY() + Parameter::CATEGORY_SEP + "refine" } ;
	
	return category;
};

/////////////////////////////////////////////////////////////

ArucoParametersRegistry::ArucoParametersRegistry()
	: ParameterRegistry()
{
	registerAllParameters();
}

/////////////////////////////////////////////////////////////

void ArucoParametersRegistry::registerAllParameters()
{
	// Marker length
	{
		std::shared_ptr<Parameter> aParam = ParameterDouble::create(
			"marker_length",
			ArucoParametersRegistry::CATEGORY_GEOMETRY(),
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			7.0,
			0.0,
			std::nullopt,
			std::nullopt,
			"Marker length",
			"mm",
			"Specifies the physical size of one marker on the ArUco pattern, expressed in real-world units. The unit must match the one used for calibration outputs."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}

	// Adaptive Thresh Win Size Min
	{
		std::shared_ptr<Parameter> aParam = ParameterInt::create(
			"adaptive_thresh_win_size_min",
			ArucoParametersRegistry::CATEGORY_DETECTION(),
			AccessMode::READWRITE,
			VisibilityLevel::ADVANCED,
			ArucoParameters::Detection::Defaults::arucoDetector.adaptiveThreshWinSizeMin,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			"Adaptive thresh win size min",
			std::nullopt,
			"Minimum window size for adaptive thresholding before finding contours."
		);

		registerParameter(aParam->getParameterView());
	}

	// Adaptive Thresh Win Size Max
	{
		std::shared_ptr<Parameter> aParam = ParameterInt::create(
			"adaptive_thresh_win_size_max",
			ArucoParametersRegistry::CATEGORY_DETECTION(),
			AccessMode::READWRITE,
			VisibilityLevel::ADVANCED,
			ArucoParameters::Detection::Defaults::arucoDetector.adaptiveThreshWinSizeMax,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			"Adaptive thresh win size max",
			std::nullopt,
			"Maximum window size for adaptive thresholding before finding contours."
		);

		registerParameter(aParam->getParameterView());
	}

	// Adaptive Thresh Win Size Step
	{
		std::shared_ptr<Parameter> aParam = ParameterInt::create(
			"adaptive_thresh_win_size_step",
			ArucoParametersRegistry::CATEGORY_DETECTION(),
			AccessMode::READWRITE,
			VisibilityLevel::ADVANCED,
			ArucoParameters::Detection::Defaults::arucoDetector.adaptiveThreshWinSizeStep,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			"Adaptive thresh win size step",
			std::nullopt,
			"Increments from adaptiveThreshWinSizeMin to adaptiveThreshWinSizeMax during the thresholding."
		);

		registerParameter(aParam->getParameterView());
	}

	// Adaptive Thresh Constant
	{
		std::shared_ptr<Parameter> aParam = ParameterDouble::create(
			"adaptive_thresh_constant",
			ArucoParametersRegistry::CATEGORY_DETECTION(),
			AccessMode::READWRITE,
			VisibilityLevel::ADVANCED,
			ArucoParameters::Detection::Defaults::arucoDetector.adaptiveThreshConstant,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			"Adaptive thresh constant",
			std::nullopt,
			"Constant for adaptive thresholding before finding contours."
		);

		registerParameter(aParam->getParameterView());
	}

	// Min Marker Perimeter Rate
	{
		std::shared_ptr<Parameter> aParam = ParameterDouble::create(
			"min_marker_perimeter_rate",
			ArucoParametersRegistry::CATEGORY_DETECTION(),
			AccessMode::READWRITE,
			VisibilityLevel::ADVANCED,
			ArucoParameters::Detection::Defaults::arucoDetector.minMarkerPerimeterRate,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			"Min marker perimeter rate",
			std::nullopt,
			"Determine minimum perimeter for marker contour to be detected. This is defined as a rate respect to the maximum dimension of the input image."
		);

		registerParameter(aParam->getParameterView());
	}

	// Max Marker Perimeter Rate
	{
		std::shared_ptr<Parameter> aParam = ParameterDouble::create(
			"max_marker_perimeter_rate",
			ArucoParametersRegistry::CATEGORY_DETECTION(),
			AccessMode::READWRITE,
			VisibilityLevel::ADVANCED,
			ArucoParameters::Detection::Defaults::arucoDetector.maxMarkerPerimeterRate,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			"Max marker perimeter rate",
			std::nullopt,
			"Determine maximum perimeter for marker contour to be detected. This is defined as a rate respect to the maximum dimension of the input image."
		);

		registerParameter(aParam->getParameterView());
	}

	// Polygonal Approx Accuracy Rate
	{
		std::shared_ptr<Parameter> aParam = ParameterDouble::create(
			"polygonal_approx_accuracy_rate",
			ArucoParametersRegistry::CATEGORY_DETECTION(),
			AccessMode::READWRITE,
			VisibilityLevel::ADVANCED,
			ArucoParameters::Detection::Defaults::arucoDetector.polygonalApproxAccuracyRate,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			"Polygonal approx accuracy rate",
			std::nullopt,
			"Minimum accuracy during the polygonal approximation process to determine which contours are squares."
		);

		registerParameter(aParam->getParameterView());
	}

	// Min Corner Distance Rate
	{
		std::shared_ptr<Parameter> aParam = ParameterDouble::create(
			"min_corner_distance_rate",
			ArucoParametersRegistry::CATEGORY_DETECTION(),
			AccessMode::READWRITE,
			VisibilityLevel::ADVANCED,
			ArucoParameters::Detection::Defaults::arucoDetector.minCornerDistanceRate,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			"Min corner distance rate",
			std::nullopt,
			"Minimum distance between corners for detected markers relative to its perimeter."
		);

		registerParameter(aParam->getParameterView());
	}

	// Min Distance To Border
	{
		std::shared_ptr<Parameter> aParam = ParameterInt::create(
			"min_distance_to_border",
			ArucoParametersRegistry::CATEGORY_DETECTION(),
			AccessMode::READWRITE,
			VisibilityLevel::ADVANCED,
			ArucoParameters::Detection::Defaults::arucoDetector.minDistanceToBorder,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			"Min distance to border",
			"px",
			"Minimum distance of any corner to the image border for detected markers."
		);

		registerParameter(aParam->getParameterView());
	}

	// Min Marker Distance Rate
	{
		std::shared_ptr<Parameter> aParam = ParameterDouble::create(
			"min_marker_distance_rate",
			ArucoParametersRegistry::CATEGORY_DETECTION(),
			AccessMode::READWRITE,
			VisibilityLevel::ADVANCED,
			ArucoParameters::Detection::Defaults::arucoDetector.minMarkerDistanceRate,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			"Min marker distance rate",
			std::nullopt,
			"Minimum average distance between the corners of the two markers to be grouped (default 0.125). The rate is relative to the smaller perimeter of the two markers. Two markers are grouped if average distance between the corners of the two markers is less than min(MarkerPerimeter1, MarkerPerimeter2)*minMarkerDistanceRate. Default value is 0.125 because 0.125*MarkerPerimeter = (MarkerPerimeter / 4) * 0.5 = half the side of the marker. Note: default value was changed from 0.05 after 4.8.1 release, because the filtering algorithm has been changed. Now a few candidates from the same group can be added to the list of candidates if they are far from each other."
		);

		registerParameter(aParam->getParameterView());
	}

	// Min Group Distance
	{
		std::shared_ptr<Parameter> aParam = ParameterDouble::create(
			"min_group_distance",
			ArucoParametersRegistry::CATEGORY_DETECTION(),
			AccessMode::READWRITE,
			VisibilityLevel::ADVANCED,
			ArucoParameters::Detection::Defaults::arucoDetector.minGroupDistance,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			"Min group distance",
			std::nullopt,
			"Minimum average distance between the corners of the two markers in group to add them to the list of candidates. The average distance between the corners of the two markers is calculated relative to its module size."
		);

		registerParameter(aParam->getParameterView());
	}

	// Corner Refinement Win Size
	{
		std::shared_ptr<Parameter> aParam = ParameterInt::create(
			"corner_refinement_win_size",
			ArucoParametersRegistry::CATEGORY_DETECTION(),
			AccessMode::READWRITE,
			VisibilityLevel::ADVANCED,
			ArucoParameters::Detection::Defaults::arucoDetector.cornerRefinementWinSize,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			"Corner refinement win size",
			std::nullopt,
			"Maximum window size for the corner refinement process (in pixels) (default 5). The window size may decrease if the ArUco marker is too small, check relativeCornerRefinmentWinSize. The final window size is calculated as: min(cornerRefinementWinSize, averageArucoModuleSize*relativeCornerRefinmentWinSize), where averageArucoModuleSize is average module size of ArUco marker in pixels (ArUco marker is composed of black and white modules)."
		);

		registerParameter(aParam->getParameterView());
	}

	// Relative Corner Refinement Win Size
	{
		std::shared_ptr<Parameter> aParam = ParameterDouble::create(
			"relative_corner_refinement_win_size",
			ArucoParametersRegistry::CATEGORY_DETECTION(),
			AccessMode::READWRITE,
			VisibilityLevel::ADVANCED,
			ArucoParameters::Detection::Defaults::arucoDetector.relativeCornerRefinmentWinSize,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			"Relative corner refinement win size",
			std::nullopt,
			"Dynamic window size for corner refinement relative to Aruco module size (default 0.3). The final window size is calculated as: min(cornerRefinementWinSize, averageArucoModuleSize*relativeCornerRefinmentWinSize), where averageArucoModuleSize is average module size of ArUco marker in pixels (ArUco marker is composed of black and white modules). In the case of markers located far from each other, it may be useful to increase the value of the parameter to 0.4-0.5. In the case of markers located close to each other, it may be useful to decrease the parameter value to 0.1-0.2."
		);

		registerParameter(aParam->getParameterView());
	}

	// Corner Refinement Max Iterations
	{
		std::shared_ptr<Parameter> aParam = ParameterInt::create(
			"corner_refinement_max_iterations",
			ArucoParametersRegistry::CATEGORY_DETECTION(),
			AccessMode::READWRITE,
			VisibilityLevel::ADVANCED,
			ArucoParameters::Detection::Defaults::arucoDetector.cornerRefinementMaxIterations,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			"Corner refinement max iterations",
			std::nullopt,
			"Maximum number of iterations for stop criteria of the corner refinement process."
		);

		registerParameter(aParam->getParameterView());
	}

	// Corner Refinement Min Accuracy
	{
		std::shared_ptr<Parameter> aParam = ParameterDouble::create(
			"corner_refinement_min_accuracy",
			ArucoParametersRegistry::CATEGORY_DETECTION(),
			AccessMode::READWRITE,
			VisibilityLevel::ADVANCED,
			ArucoParameters::Detection::Defaults::arucoDetector.cornerRefinementMinAccuracy,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			"Corner refinement min accuracy",
			std::nullopt,
			"Minimum error for the stop criteria of the corner refinement process."
		);

		registerParameter(aParam->getParameterView());
	}

	// Marker Border Bits
	{
		std::shared_ptr<Parameter> aParam = ParameterInt::create(
			"marker_border_bits",
			ArucoParametersRegistry::CATEGORY_DETECTION(),
			AccessMode::READWRITE,
			VisibilityLevel::ADVANCED,
			ArucoParameters::Detection::Defaults::arucoDetector.markerBorderBits,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			"Marker border bits",
			std::nullopt,
			"Number of bits of the marker border, i.e. marker border width."
		);

		registerParameter(aParam->getParameterView());
	}

	// Perspective Remove Pixel Per Cell
	{
		std::shared_ptr<Parameter> aParam = ParameterInt::create(
			"perspective_remove_pixel_per_cell",
			ArucoParametersRegistry::CATEGORY_DETECTION(),
			AccessMode::READWRITE,
			VisibilityLevel::ADVANCED,
			ArucoParameters::Detection::Defaults::arucoDetector.perspectiveRemovePixelPerCell,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			"Perspective remove pixel per cell",
			std::nullopt,
			"Number of bits (per dimension) for each cell of the marker when removing the perspective."
		);

		registerParameter(aParam->getParameterView());
	}

	// Perspective Remove Ignored Margin Per Cell
	{
		std::shared_ptr<Parameter> aParam = ParameterDouble::create(
			"perspective_remove_ignored_margin_per_cell",
			ArucoParametersRegistry::CATEGORY_DETECTION(),
			AccessMode::READWRITE,
			VisibilityLevel::ADVANCED,
			ArucoParameters::Detection::Defaults::arucoDetector.perspectiveRemoveIgnoredMarginPerCell,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			"Perspective remove ignored margin per cell",
			std::nullopt,
			"Width of the margin of pixels on each cell not considered for the determination of the cell bit. Represents the rate respect to the total size of the cell, i.e. perspectiveRemovePixelPerCell"
		);

		registerParameter(aParam->getParameterView());
	}

	// Max Erroneous Bits In Border Rate
	{
		std::shared_ptr<Parameter> aParam = ParameterDouble::create(
			"max_erroneous_bits_in_border_rate",
			ArucoParametersRegistry::CATEGORY_DETECTION(),
			AccessMode::READWRITE,
			VisibilityLevel::ADVANCED,
			ArucoParameters::Detection::Defaults::arucoDetector.maxErroneousBitsInBorderRate,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			"Max erroneous bits in border rate",
			std::nullopt,
			"Maximum number of accepted erroneous bits in the border (i.e. number of allowed white bits in the border). Represented as a rate respect to the total number of bits per marker."
		);

		registerParameter(aParam->getParameterView());
	}

	// Min Otsu Std Dev
	{
		std::shared_ptr<Parameter> aParam = ParameterDouble::create(
			"min_otsu_std_dev",
			ArucoParametersRegistry::CATEGORY_DETECTION(),
			AccessMode::READWRITE,
			VisibilityLevel::ADVANCED,
			ArucoParameters::Detection::Defaults::arucoDetector.minOtsuStdDev,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			"Min otsu std dev",
			std::nullopt,
			"Minimum standard deviation in pixels values during the decodification step to apply Otsu thresholding (otherwise, all the bits are set to 0 or 1 depending on mean higher than 128 or not)."
		);

		registerParameter(aParam->getParameterView());
	}

	// Error Correction Rate
	{
		std::shared_ptr<Parameter> aParam = ParameterDouble::create(
			"error_correction_rate",
			ArucoParametersRegistry::CATEGORY_DETECTION(),
			AccessMode::READWRITE,
			VisibilityLevel::ADVANCED,
			ArucoParameters::Detection::Defaults::arucoDetector.errorCorrectionRate,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			"Error correction rate",
			std::nullopt,
			"Error correction rate respect to the maximum error correction capability for each dictionary."
		);

		registerParameter(aParam->getParameterView());
	}

	// Detect Inverted Marker
	{
		std::shared_ptr<Parameter> aParam = ParameterBool::create(
			"detect_inverted_marker",
			ArucoParametersRegistry::CATEGORY_DETECTION(),
			AccessMode::READWRITE,
			VisibilityLevel::ADVANCED,
			ArucoParameters::Detection::Defaults::arucoDetector.detectInvertedMarker,
			"Detect inverted marker",
			std::nullopt,
			"To check if there is a white marker. In order to generate a 'white' marker just invert a normal marker by using a tilde, ~markerImage."
		);

		registerParameter(aParam->getParameterView());
	}

	// Use Aruco 3 Detection
	{
		std::shared_ptr<Parameter> aParam = ParameterBool::create(
			"use_aruco3_detection",
			ArucoParametersRegistry::CATEGORY_DETECTION(),
			AccessMode::READWRITE,
			VisibilityLevel::ADVANCED,
			ArucoParameters::Detection::Defaults::arucoDetector.useAruco3Detection,
			"Use aruco 3 detection",
			std::nullopt,
			"Enable the new and faster Aruco detection strategy. Proposed in the paper: Romero-Ramirez et al: Speeded up detection of squared fiducial markers (2018) https://www.researchgate.net/publication/325787310_Speeded_Up_Detection_of_Squared_Fiducial_Markers"
		);

		registerParameter(aParam->getParameterView());
	}

	// Min Side Length Canonical Img
	{
		std::shared_ptr<Parameter> aParam = ParameterInt::create(
			"min_side_length_canonical_img",
			ArucoParametersRegistry::CATEGORY_DETECTION(),
			AccessMode::READWRITE,
			VisibilityLevel::ADVANCED,
			ArucoParameters::Detection::Defaults::arucoDetector.minSideLengthCanonicalImg,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			"Min side length canonical img",
			std::nullopt,
			"Minimum side length of a marker in the canonical image. Latter is the binarized image in which contours are searched."
		);

		registerParameter(aParam->getParameterView());
	}

	// Min Marker Length Ratio Original Img
	{
		std::shared_ptr<Parameter> aParam = ParameterDouble::create(
			"min_marker_length_ratio_original_img",
			ArucoParametersRegistry::CATEGORY_DETECTION(),
			AccessMode::READWRITE,
			VisibilityLevel::ADVANCED,
			ArucoParameters::Detection::Defaults::arucoDetector.minMarkerLengthRatioOriginalImg,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			"Min marker length ratio original img",
			std::nullopt,
			"Range [0,1], eq (2) from paper. The parameter tau_i has a direct influence on the processing speed."
		);

		registerParameter(aParam->getParameterView());
	}

	// Min Rep Distance
	{
		std::shared_ptr<Parameter> aParam = ParameterDouble::create(
			"min_rep_distance",
			ArucoParametersRegistry::CATEGORY_REFINE(),
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			ArucoParameters::Refine::Defaults::arucoRefine.minRepDistance,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			"Min rep distance",
			std::nullopt,
			"Minimum distance between the corners of the rejected candidate and the reprojected marker in order to consider it as a correspondence."
		);

		registerParameter(aParam->getParameterView());
	}

	// Error Correction Rate
	{
		std::shared_ptr<Parameter> aParam = ParameterDouble::create(
			"error_correction_rate",
			ArucoParametersRegistry::CATEGORY_REFINE(),
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			ArucoParameters::Refine::Defaults::arucoRefine.errorCorrectionRate,
			-1,
			std::nullopt,
			std::nullopt,
			"Error correction rate",
			std::nullopt,
			"Rate of allowed erroneous bits respect to the error correction capability of the used dictionary. -1 ignores the error correction step."
		);

		registerParameter(aParam->getParameterView());
	}

	// Check All Orders
	{
		std::shared_ptr<Parameter> aParam = ParameterBool::create(
			"check_all_orders",
			ArucoParametersRegistry::CATEGORY_REFINE(),
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			ArucoParameters::Refine::Defaults::arucoRefine.checkAllOrders,
			"Check all orders",
			std::nullopt,
			"Consider the four posible corner orders in the rejectedCorners array. If it set to false, only the provided corner order is considered."
		);

		registerParameter(aParam->getParameterView());
	}
}
