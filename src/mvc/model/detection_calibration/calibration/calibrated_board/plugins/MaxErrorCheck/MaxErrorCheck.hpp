#pragma once
#include "../CalibratedBoardPlugin.hpp"


/**
 * @brief Plugin for validating the max reprojection error on a calibrated board.
 *
 * score = 1.0 - ((max board Error) / maxMaxError)
 */
class MaxErrorCheck : public Plugin<CalibratedBoard>
{
public:
    //-----------------------------------------------------------------------------
    // Structs and constants
    //-----------------------------------------------------------------------------

    /** @brief Default values for the plugin parameters */
    struct Defaults {
        static constexpr double maxMaxError{ 2.0 };   // pixels
    };

    static constexpr std::string_view ID{ "max_reproj_error" };
    static constexpr std::string_view NAME{ "Reprojection Error Check" };
    static constexpr std::string_view DESCRIPTION{
        R"(Checks Max reprojection error after calibration against configurable Max threshold.
        score = 1.0 - ((max board Error) / maxMaxError)
        )"
    };


    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    ~MaxErrorCheck() = default;


    //-----------------------------------------------------------------------------
    // Construction from factories
    //-----------------------------------------------------------------------------

    /**
     * @brief Factory method to create a plugin instance.
     * @param threshold Initial score threshold.
     * @return A shared pointer to the newly created plugin.
     */
    static std::shared_ptr<MaxErrorCheck> create(double threshold = DEFAULT_THRESHOLD);

private:
    // Private constructor to ensure instantiation via factory methods.
    MaxErrorCheck(double threshold);

    std::shared_ptr<PluginResult> executeImpl(
        const CalibratedBoard& calibratedBoard,
        const std::unordered_map<std::string, std::shared_ptr<PluginResult>>& producersResults = {}
    ) const override;

    void validateParameters() const override;

    std::string getMessageForSeverity(EvaluationSeverity severity) const override;

public:
    // Threshold above which score is clamped to 0. Valid range: > 0.0
    double maxMaxError{ Defaults::maxMaxError };
};
