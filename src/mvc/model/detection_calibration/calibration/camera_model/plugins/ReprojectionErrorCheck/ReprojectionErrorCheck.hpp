#pragma once
#include "../CameraModelPlugin.hpp"


/**
 * @brief Plugin for validating the max reprojection error on a calibrated board.
 *
 * score = 1.0 - ((max camera model Error) / maxError)
 */
class ReprojectionErrorCheck : public Plugin<CameraModel>
{
public:
    //-----------------------------------------------------------------------------
    // Structs and constants
    //-----------------------------------------------------------------------------

    /** @brief Default values for the plugin parameters */
    struct Defaults {
        static constexpr double maxError{ 1.0 };   // pixels
    };

    static constexpr std::string_view ID{ "max_RMS_error" };
    static constexpr std::string_view NAME{ "Camera Model Reprojection Error Check" };
    static constexpr std::string_view DESCRIPTION{
        R"(Checks Max reprojection error after calibration against configurable Max threshold.
        score = 1.0 - ((max camera model Error) / maxError)
        )"
    };


    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    ~ReprojectionErrorCheck() = default;


    //-----------------------------------------------------------------------------
    // Construction from factories
    //-----------------------------------------------------------------------------

    /**
     * @brief Factory method to create a plugin instance.
     * @param threshold Initial score threshold.
     * @return A shared pointer to the newly created plugin.
     */
    static std::shared_ptr<ReprojectionErrorCheck> create(double threshold = DEFAULT_THRESHOLD);

private:
    // Private constructor to ensure instantiation via factory methods.
    ReprojectionErrorCheck(double threshold);

    std::shared_ptr<PluginResult> executeImpl(const CameraModel& camModel) const override;

    void validateParameters() const override;

    std::string getMessageForSeverity(EvaluationSeverity severity) const override;

public:
    // Threshold above which score is clamped to 0. Valid range: > 0.0
    double maxError{ Defaults::maxError };
};
