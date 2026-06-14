#include "BoardSizeCheckRegistry.hpp"
#include <parameter/numeric/ParameterNumeric.hpp>
#include "BoardSizeCheck.hpp"


const std::string BoardSizeCheckRegistry::CATEGORY{ BoardSizeCheck::ID };

BoardSizeCheckRegistry::BoardSizeCheckRegistry()
	: ParameterRegistry()
{
	registerAllParameters();
}

/////////////////////////////////////////////////////////////

void BoardSizeCheckRegistry::registerAllParameters()
{
	// Expected coverage
	{
		std::shared_ptr<Parameter> aParam = ParameterDouble::create(
			"expected_coverage",
			CATEGORY,
			AccessMode::READWRITE,
			VisibilityLevel::BASIC,
			0.2,
			0.05,
			0.5,
			0.01,
			"Expected coverage",
			std::nullopt,
			"Specifies the fraction of the image area that a correctly-sized board should occupy. Scores are normalized so that a board covering exactly this fraction receives 100%, while smaller boards receive proportionally lower scores. Lowering this value makes the plugin more tolerant of small boards, while increasing it rewards boards that occupy more of the image."
		);

		registerParameter(
			aParam->getParameterView()
		);
	}
}
