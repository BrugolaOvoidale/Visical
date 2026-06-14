#pragma once
#include "../CalibratedBoardPlugin.hpp"


/**
 * @brief Plugin for validating the max reprojection error on a calibrated board.
 *
 * score = 1.0 - ((max board RMS Error) / maxRMSError)
 */
class RMSErrorCheck : public Plugin<CalibratedBoard>
{
public:
    //-----------------------------------------------------------------------------
    // Structs and constants
    //-----------------------------------------------------------------------------

    /** @brief Default values for the plugin parameters */
    struct Defaults {
        static constexpr double maxRMSError{ 1.0 };   // pixels
    };

    static constexpr std::string_view ID{ "RMS_reproj_error" };
    static constexpr std::string_view NAME{ "RMS Reprojection Error Check" };
    static constexpr std::string_view DESCRIPTION{
        R"(Checks RMS reprojection after calibration against configurable Max threshold.
        score = 1.0 - ((max board Error) / maxMaxError)
        )"
    };

    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    ~RMSErrorCheck() = default;


    //-----------------------------------------------------------------------------
    // Construction from factories
    //-----------------------------------------------------------------------------

    /**
     * @brief Factory method to create a plugin instance.
     * @param threshold Initial score threshold.
     * @return A shared pointer to the newly created plugin.
     */
    static std::shared_ptr<RMSErrorCheck> create(double threshold = DEFAULT_THRESHOLD);

private:
    // Private constructor to ensure instantiation via factory methods.
    RMSErrorCheck(double threshold);

    std::shared_ptr<PluginResult> executeImpl(const CalibratedBoard& calibratedBoard) const override;

    void validateParameters() const override;

    std::string getMessageForSeverity(EvaluationSeverity severity) const override;

public:
    // Threshold above which score is clamped to 0. Valid range: > 0.0
    double maxRMSError{ Defaults::maxRMSError };
};
