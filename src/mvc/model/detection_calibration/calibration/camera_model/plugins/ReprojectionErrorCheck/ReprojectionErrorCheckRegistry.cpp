#include <parameter/numeric/ParameterNumeric.hpp>
#include "ReprojectionErrorCheckRegistry.hpp"
#include "ReprojectionErrorCheck.hpp"


const std::string ReprojectionErrorCheckRegistry::CATEGORY{ ReprojectionErrorCheck::ID };

ReprojectionErrorCheckRegistry::ReprojectionErrorCheckRegistry()
	: ParameterRegistry()
{
	registerAllParameters();
}

/////////////////////////////////////////////////////////////

void ReprojectionErrorCheckRegistry::registerAllParameters()
{
    // Max RMS error
    {
        std::shared_ptr<Parameter> aParam = ParameterDouble::create(
            "max_error",
            CATEGORY,
            AccessMode::READWRITE,
            VisibilityLevel::BASIC,
            ReprojectionErrorCheck::Defaults::maxError,
            0.1,
            20.0,
            0.1,
            "Max error",
            "px",
            "Maximum acceptable global RMS reprojection error in pixels across all calibrated boards. Quality is normalized so that an error of zero scores 100 and an error equal to this threshold scores 0, with linear interpolation in between. This reflects the overall fit of the camera model to the entire calibration sequence, as opposed to the per-board reprojection error check. Lowering this value enforces stricter calibration quality, while raising it makes the check more tolerant of higher overall error."
        );

        registerParameter(
            aParam->getParameterView()
        );
    }
}
