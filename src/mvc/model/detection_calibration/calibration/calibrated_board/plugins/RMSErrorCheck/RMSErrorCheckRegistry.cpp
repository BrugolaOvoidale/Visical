#include "RMSErrorCheckRegistry.hpp"
#include <parameter/numeric/ParameterNumeric.hpp>
#include "RMSErrorCheck.hpp"


const std::string RMSErrorCheckRegistry::CATEGORY{ RMSErrorCheck::ID };

RMSErrorCheckRegistry::RMSErrorCheckRegistry()
	: ParameterRegistry()
{
	registerAllParameters();
}

/////////////////////////////////////////////////////////////

void RMSErrorCheckRegistry::registerAllParameters()
{
    // Max RMS error
    {
        std::shared_ptr<Parameter> aParam = ParameterDouble::create(
            "max_rms_error",
            CATEGORY,
            AccessMode::READWRITE,
            VisibilityLevel::BASIC,
            RMSErrorCheck::Defaults::maxRMSError,
            0.1,
            10.0,
            0.1,
            "Max RMS error",
            "px",
            "Maximum acceptable per-board RMS reprojection error in pixels. Quality is normalized so that an error of zero scores 100 and an error equal to this threshold scores 0, with linear interpolation in between. Lowering this value enforces stricter quality requirements, while raising it makes the check more tolerant of boards with higher reprojection error."
        );

        registerParameter(
            aParam->getParameterView()
        );
    }
}
