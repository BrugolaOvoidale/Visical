#pragma once
#include "FOVCoverageCheckDebug.hpp"
#include "../BoardSequencePlugin.hpp"


/**
 * @brief Plugin for validating spatial distribution of observation points.
 *
 * This plugin analyzes a sequence of Board objects to determine if the detected
 * points sufficiently cover the Field of View (FOV). It calculates a score based on
 * normalized coverage and applies specific thresholds to determine evaluation severity.
 */
class FOVCoverageCheck : public Plugin<std::vector<std::shared_ptr<Board>>, FOVCoverageCheckDebug>
{
public:
    //-----------------------------------------------------------------------------
    // Structs and constants
    //-----------------------------------------------------------------------------

    /** @brief Default values for the plugin parameters */
    struct Defaults {
        static constexpr double borderExclusionRatio{ 0.067 };
        static constexpr double coverageNormalizationFactor{ 2.5 };
        static constexpr double minCoverageThreshold{ 0.3 };
        static constexpr double maxCoverageThreshold{ 0.85 };
    };

    static constexpr std::string_view ID { "fov_coverage" };
    static constexpr std::string_view NAME { "FOV Coverage Check" };
    static constexpr std::string_view DESCRIPTION { "Validates field-of-view coverage of observation points" };


    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    ~FOVCoverageCheck() = default;


    //-----------------------------------------------------------------------------
    // Construction from factories
    //-----------------------------------------------------------------------------

    /**
     * @brief Factory method to create a plugin instance.
     * @param threshold Initial score threshold.
     * @return A shared pointer to the newly created plugin.
     */
    static std::shared_ptr<FOVCoverageCheck> create(double threshold = DEFAULT_THRESHOLD);

private:
    // Private constructor to ensure instantiation via factory methods.
    FOVCoverageCheck(double threshold);

    std::shared_ptr<PluginResult> executeImpl(const std::vector<std::shared_ptr<Board>>& boards) const override;

    void validateParameters() const override;

    std::string getMessageForSeverity(EvaluationSeverity severity) const override;

public:
    // Ratio of the image border to ignore
    double borderExclusionRatio{ Defaults::borderExclusionRatio };

    // Coefficient used to map raw coverage to a [0, 1] score range.
    double coverageNormalizationFactor{ Defaults::coverageNormalizationFactor };

    // Scores below this value are considered critical failures.
    double minCoverageThreshold{ Defaults::minCoverageThreshold };

    // Scores above this value are considered optimal.
    double maxCoverageThreshold{ Defaults::maxCoverageThreshold };

private:
    static inline const CvImage BLANK_HEATMAP{ cv::Mat(100, 100, CV_8UC3, 0.0) };
};
