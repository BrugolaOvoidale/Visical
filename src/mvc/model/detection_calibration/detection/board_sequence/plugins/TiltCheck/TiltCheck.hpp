#pragma once
#include "TiltCheckDebug.hpp"
#include "../BoardSequencePlugin.hpp"


/**
 * @brief Evaluates the geometric diversity of a board sequence based on camera pose.
 *
 * This plugin analyzes the variation in Slant and Pan angles across multiple images.
 * A high score indicates that the boards were captured from a wide range of angles.
 */
class TiltCheck : public Plugin<std::vector<std::shared_ptr<Board>>>
{
public:
    //-----------------------------------------------------------------------------
    // Structs and constants
    //-----------------------------------------------------------------------------

    /** @brief Default values for the plugin parameters */
    struct Defaults{
        static constexpr double maxAngleCap{ 75.0 };
        static constexpr double angleScalingTarget{ 90.0 };
        static constexpr double scoreExponent{ 90.0 };
        static constexpr double scoreAdjustmentMultiplier{ 1.05 };
        static constexpr size_t minImages{ 4 };
        static constexpr double slantWeight{ 0.5 };
        static constexpr double panWeight{ 0.5 };
    };

    static constexpr std::string_view ID{ "tilt" };
    static constexpr std::string_view NAME{ "Tilt Check" };
    static constexpr std::string_view DESCRIPTION{ "Analyzes camera pose variation across images" };


    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    ~TiltCheck() = default;


    //-----------------------------------------------------------------------------
    // Construction from factories
    //-----------------------------------------------------------------------------

    /**
     * @brief Factory method to create a plugin instance.
     * @param threshold Initial score threshold.
     * @return A shared pointer to the newly created plugin.
     */
    static std::shared_ptr<TiltCheck> create(double threshold = DEFAULT_THRESHOLD);


private:
    // Private constructor to ensure instantiation via factory methods.
    TiltCheck(double threshold);

    std::shared_ptr<PluginResult> executeImpl(const std::vector<std::shared_ptr<Board>>& boards) const override;

    void validateParameters() const override;

    /**
     * @brief Decomposes a rotation matrix into Euler angles.
     * @param[in] R The 3x3 rotation matrix.
     * @param[out] slant Rotation around the X-axis.
     * @param[out] pan Rotation around the Y-axis.
     * @param[out] roll Rotation around the Z-axis.
     */
    void extractEulerAngles(
        const cv::Mat& R,
        double& slant,
        double& pan,
        double& roll
    ) const;

    /** @brief Maps a normalized angle value to a 0-1 score using a non-linear exponent. */
    double computeSingleScore(double normalizedAngle) const;

    std::string getMessageForSeverity(EvaluationSeverity severity) const override;

public:
    // Maximum angle (degrees) considered useful.
    double maxAngleCap{ Defaults::maxAngleCap };

    // Normalization factor for angle distribution.
    double angleScalingTarget{ Defaults::angleScalingTarget };

    // Steepness of the scoring decay curve.
    double scoreExponent{ Defaults::scoreExponent };

    // Buffer for threshold comparisons.
    double scoreAdjustmentMultiplier{ Defaults::scoreAdjustmentMultiplier };

    // Minimum images required for a valid evaluation.
    size_t minImages{ Defaults::minImages };

    // Influence of Slant variation on final score.
    double slantWeight{ Defaults::slantWeight };

    // Influence of Pan variation on final score.
    double panWeight{ Defaults::panWeight };
};
