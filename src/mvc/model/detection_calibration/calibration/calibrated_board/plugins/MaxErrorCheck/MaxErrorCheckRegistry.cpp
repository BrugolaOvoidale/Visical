#include "MaxErrorCheckRegistry.hpp"
#include <parameter/numeric/ParameterNumeric.hpp>
#include "MaxErrorCheck.hpp"


const std::string MaxErrorCheckRegistry::CATEGORY{ MaxErrorCheck::ID };

MaxErrorCheckRegistry::MaxErrorCheckRegistry()
	: ParameterRegistry()
{
	registerAllParameters();
}

/////////////////////////////////////////////////////////////

void MaxErrorCheckRegistry::registerAllParameters()
{
    // Max RMS error
    {
        std::shared_ptr<Parameter> aParam = ParameterDouble::create(
            "max_error",
            CATEGORY,
            AccessMode::READWRITE,
            VisibilityLevel::BASIC,
            MaxErrorCheck::Defaults::maxMaxError,
            0.1,
            20.0,
            0.1,
            "Max point error",
            "px",
            "Maximum acceptable per-point reprojection error in pixels for any single corner on the board. This catches boards where calibration produced a plausible average but with severe local outliers. Should generally be set higher than Max RMS error."
        );

        registerParameter(
            aParam->getParameterView()
        );
    }
}
