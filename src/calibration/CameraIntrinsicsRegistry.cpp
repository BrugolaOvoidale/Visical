#include "CameraIntrinsicsRegistry.hpp"
#include <parameter/numeric/ParameterNumeric.hpp>


CameraIntrinsicsRegistry::CameraIntrinsicsRegistry()
	: ParameterRegistry()
{
	registerAllParameters();
}

/////////////////////////////////////////////////////////////

void CameraIntrinsicsRegistry::registerAllParameters()
{
	// Image width
	{
		std::shared_ptr<ParameterInt> aParam = ParameterInt::create(
			"image_width",
			CATEGORY,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			100,
			0,
			std::nullopt,
			std::nullopt,
			"Image width",
			"px",
			"Width of the captured image in pixels."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}

	// Image height
	{
		std::shared_ptr<ParameterInt> aParam = ParameterInt::create(
			"image_height",
			CATEGORY,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			100,
			0,
			std::nullopt,
			std::nullopt,
			"Image height",
			"px",
			"Height of the captured image in pixels."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}

	// Focal length
	{
		std::shared_ptr<ParameterDouble> aParam = ParameterDouble::create(
			"focal_length",
			CATEGORY,
			AccessMode::READWRITE,
			VisibilityLevel::INTERMEDIATE,
			12.5,
			0.0,
			std::nullopt,
			std::nullopt,
			"Focal length",
			"mm",
			"Physical focal length of the camera lens in millimeters. Converted to pixel units using the sensor pixel pitch to build the camera intrinsic matrix."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}

	// Pixel pitch width
	{
		std::shared_ptr<ParameterDouble> aParam = ParameterDouble::create(
			"pixel_pitch_width",
			CATEGORY,
			AccessMode::READWRITE,
			VisibilityLevel::INTERMEDIATE,
			0.00345,
			0.0,
			std::nullopt,
			0.00001,
			"Pixel pitch width",
			"mm",
			"Width of a single sensor pixel in millimeters. Used to convert the focal length from millimeters to pixels."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}

	// Pixel pitch height
	{
		std::shared_ptr<ParameterDouble> aParam = ParameterDouble::create(
			"pixel_pitch_height",
			CATEGORY,
			AccessMode::READWRITE,
			VisibilityLevel::INTERMEDIATE,
			0.00345,
			0.0,
			std::nullopt,
			0.00001,
			"Pixel pitch height",
			"mm",
			"Height of a single sensor pixel in millimeters. Used to convert the focal length from millimeters to pixels."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}

	// Focal Length X
	{
		std::shared_ptr<ParameterDouble> aParam = ParameterDouble::create(
			"focal_length_x",
			CATEGORY,
			AccessMode::READWRITE,
			VisibilityLevel::ADVANCED,
			1000.0,
			0.1,
			std::nullopt,
			0.01,
			"Focal Length X",
			"px",
			"Horizontal focal length in pixels."
		);
		registerParameter(aParam->getParameterView());
	}

	// Focal Length Y
	{
		std::shared_ptr<ParameterDouble> aParam = ParameterDouble::create(
			"focal_length_y",
			CATEGORY,
			AccessMode::READWRITE,
			VisibilityLevel::ADVANCED,
			1000.0,
			0.1,
			std::nullopt,
			0.01,
			"Focal Length Y",
			"px",
			"Vertical focal length in pixels."
		);
		registerParameter(aParam->getParameterView());
	}

	// Principal Point X
	{
		std::shared_ptr<ParameterDouble> aParam = ParameterDouble::create(
			"principal_point_x",
			CATEGORY,
			AccessMode::READWRITE,
			VisibilityLevel::ADVANCED,
			50.0,
			0.0,
			std::nullopt,
			0.1,
			"Principal Point X",
			"px",
			"The x-coordinate of the optical center on the image sensor."
		);
		registerParameter(aParam->getParameterView());
	}

	// Principal Point Y
	{
		std::shared_ptr<ParameterDouble> aParam = ParameterDouble::create(
			"principal_point_y",
			CATEGORY,
			AccessMode::READWRITE,
			VisibilityLevel::ADVANCED,
			50.0,
			0.0,
			std::nullopt,
			0.1,
			"Principal Point Y",
			"px",
			"The y-coordinate of the optical center on the image sensor."
		);
		registerParameter(aParam->getParameterView());
	}

	// Skew Coefficient
	{
		std::shared_ptr<ParameterDouble> aParam = ParameterDouble::create(
			"skew",
			CATEGORY,
			AccessMode::READWRITE,
			VisibilityLevel::ADVANCED,
			0.0,
			std::nullopt,
			std::nullopt,
			0.0001,
			"Skew",
			std::nullopt,
			"The skew coefficient between the x and y axes. Only used for specialized or legacy sensors."
		);
		registerParameter(aParam->getParameterView());
	}
}