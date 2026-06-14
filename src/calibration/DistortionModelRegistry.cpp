#include "DistortionModelRegistry.hpp"
#include <UtilityFunctions.hpp>
#include <parameter/enum/ParameterEnum.hpp>
#include <parameter/numeric/ParameterNumeric.hpp>
#include "DistortionModel.hpp"


/////////////////////////////////////////////////////////////

const std::string DistortionModelRegistry::CATEGORY{ "distortion_model" };

const std::string DistortionModelRegistry::CATEGORY_STANDARD{ CATEGORY + Parameter::CATEGORY_SEP + "standard" };

const std::string DistortionModelRegistry::CATEGORY_RATIONAL{ CATEGORY + Parameter::CATEGORY_SEP + "rational" };

const std::string DistortionModelRegistry::CATEGORY_THIN_PRISM{ CATEGORY + Parameter::CATEGORY_SEP + "thin_prism" };

const std::string DistortionModelRegistry::CATEGORY_TILTED{ CATEGORY + Parameter::CATEGORY_SEP + "tilted" };

/////////////////////////////////////////////////////////////

DistortionModelRegistry::DistortionModelRegistry()
	: ParameterRegistry()
{
	registerAllParameters();
}

/////////////////////////////////////////////////////////////

void DistortionModelRegistry::registerAllParameters()
{
	// Distortion type
	{
		std::shared_ptr<Parameter> aParam = ParameterEnum::create(
			"distortion_type",
			CATEGORY,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			UtilityFunctions::enumValuesToIntMap<DistortionModel::Type>(),
			UtilityFunctions::stringFromEnum<DistortionModel::Type>(DistortionModel::Type::STANDARD).value(),
			"Distortion type",
			std::nullopt,
			"Selects the camera lens distortion model by defining the number and type of distortion coefficients:\n\n"
			"- STANDARD(5 coefficients, K1–K3, P1–P2)\n"
			"- RATIONAL(8 coefficients, adds K4–K5–K6)\n"
			"- THIN_PRISM(12 coefficients, includes thin prism distortion S1–S4)\n"
			"- TILTED(14 coefficients, includes sensor tilt TauX / TauY)\n\n"
			"The choice affects how the calibration algorithm estimates lens distortion and how many parameters are used in the distortion vector."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}

	// Distortion coefficients
	{
		auto registerCoeff = [this](
			const std::string& name,
			const std::string& category,
			const std::string& displayName,
			const std::string& description)
			{
				std::shared_ptr<Parameter> aParam = ParameterDouble::create(
					name,
					category,
					AccessMode::READWRITE,
					VisibilityLevel::ADVANCED,
					0.0,
					-999999.9,
					std::nullopt,
					0.000001,
					displayName,
					std::nullopt,
					description
				);

				registerParameter(
					aParam->getParameterView()
				);
			};

		// Radial distortion
		registerCoeff("k1", CATEGORY_STANDARD, "Radial k1", "Radial distortion coefficient k1");
		registerCoeff("k2", CATEGORY_STANDARD, "Radial k2", "Radial distortion coefficient k2");
		registerCoeff("k3", CATEGORY_STANDARD, "Radial k3", "Radial distortion coefficient k3");
		registerCoeff("k4", CATEGORY_RATIONAL, "Radial k4", "Radial distortion coefficient k4");
		registerCoeff("k5", CATEGORY_RATIONAL, "Radial k5", "Radial distortion coefficient k5");
		registerCoeff("k6", CATEGORY_RATIONAL, "Radial k6", "Radial distortion coefficient k6");

		// Tangential distortion
		registerCoeff("p1", CATEGORY_STANDARD, "Tangential p1", "Tangential distortion coefficient p1");
		registerCoeff("p2", CATEGORY_STANDARD, "Tangential p2", "Tangential distortion coefficient p2");

		// Thin prism distortion
		registerCoeff("s1", CATEGORY_THIN_PRISM, "Thin prism s1", "Thin prism distortion coefficient s1");
		registerCoeff("s2", CATEGORY_THIN_PRISM, "Thin prism s2", "Thin prism distortion coefficient s2");
		registerCoeff("s3", CATEGORY_THIN_PRISM, "Thin prism s3", "Thin prism distortion coefficient s3");
		registerCoeff("s4", CATEGORY_THIN_PRISM, "Thin prism s4", "Thin prism distortion coefficient s4");

		// Tilt distortion
		registerCoeff("tx", CATEGORY_TILTED, "Tilt tx", "Tilt distortion coefficient tx");
		registerCoeff("ty", CATEGORY_TILTED, "Tilt ty", "Tilt distortion coefficient ty");
	}
}
