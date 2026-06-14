#include "ImagesCountCheckRegistry.hpp"
#include <parameter/numeric/ParameterNumeric.hpp>
#include "ImagesCountCheck.hpp"


const std::string ImagesCountCheckRegistry::CATEGORY{ ImagesCountCheck::ID };

ImagesCountCheckRegistry::ImagesCountCheckRegistry()
	: ParameterRegistry()
{
	registerAllParameters();
}

/////////////////////////////////////////////////////////////

void ImagesCountCheckRegistry::registerAllParameters()
{
	// Minimum image count
	{
		std::shared_ptr<Parameter> aParam = ParameterInt::create(
			"min_image_count",
			CATEGORY,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			ImagesCountCheck::Defaults::minImagesCount,
			2,
			std::nullopt,
			1,
			"Minimum image count",
			std::nullopt,
			"Defines the number of detected boards required to reach a full score of 100. The plugin linearly scales the quality based on the actual number of boards versus this target. Lower values make it easier to achieve a high score with fewer captures, useful for quick calibration sessions, while higher values encourage capturing more boards to improve statistical robustness and calibration accuracy."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}
}
