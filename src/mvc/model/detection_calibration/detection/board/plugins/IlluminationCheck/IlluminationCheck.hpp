#pragma once
#include "IlluminationCheckDebug.hpp"
#include "../BoardPlugin.hpp"


/**
 * @brief Plugin for validating the visual contrast between board surfaces and markers.
 *
 * This plugin calculates contrast metrics to ensure that markers are sufficiently
 * distinguishable from the background.
 *
 * Mark score:
 * 
 * mark deviation = (dilated mark max value) - (average of the max values in the dilated marker region)
 * 
 * score = scoreOffsetFactor - ((mark deviation) / intensityDeviationScale)
 *
 * 
 * Total score:
 * 
 * totalScore = scoreOffsetFactor - ((std dev) / intensityDeviationScale)
 */
class IlluminationCheck : public Plugin<std::shared_ptr<Board>, IlluminationCheckDebug>
{
public: 
    //-----------------------------------------------------------------------------
    // Structs and constants
    //-----------------------------------------------------------------------------

    /** @brief Default values for the plugin parameters */
    struct Defaults {
        static constexpr double markDilationRadius{ 5.5 };
        static constexpr double intensityDeviationScale{ 40.0 };
        static constexpr double scoreOffsetFactor{ 1.0 };
    };

    static constexpr std::string_view ID{ "illumination" };
    static constexpr std::string_view NAME{ "Illumination Check" };
    static constexpr std::string_view DESCRIPTION{
        R"(Checks for uniform illumination across the board
        Mark score:
        mark deviation = (dilated mark max value) - (average of the max values in the dilated marker region)
        score = scoreOffsetFactor - ((mark deviation) / intensityDeviationScale)

        Total score:
        totalScore = scoreOffsetFactor - ((std dev) / intensityDeviationScale)
        )"
    };


    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    ~IlluminationCheck() = default;


    //-----------------------------------------------------------------------------
    // Construction from factories
    //-----------------------------------------------------------------------------

    /**
     * @brief Factory method to create a plugin instance.
     * @param threshold Initial score threshold.
     * @return A shared pointer to the newly created plugin.
     */
    static std::shared_ptr<IlluminationCheck> create(double threshold = DEFAULT_THRESHOLD);

private:
    // Private constructor to ensure instantiation via factory methods.
    IlluminationCheck(double threshold);

    std::shared_ptr<PluginResult> executeImpl(const std::shared_ptr<Board>& board) const override;

    void validateParameters() const override;

    std::string getMessageForSeverity(EvaluationSeverity severity) const override;

public:
    // Dilation radius applied to marks when creating the sampling ROI. Valid range: > 0.0
    double markDilationRadius{ Defaults::markDilationRadius };

    // Scaling factor used to normalize raw intensity deviations into a score. Valid range: > 0.0
    double intensityDeviationScale{ Defaults::intensityDeviationScale };

    // The maximum possible score before subtractions. Valid range: > 0.0
    double scoreOffsetFactor{ Defaults::scoreOffsetFactor };
};
