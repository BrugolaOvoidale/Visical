#pragma once
#include "IlluminationCheckDebug.hpp"
#include "../BoardPlugin.hpp"


/**
 * @brief Plugin for validating illumination uniformity across the board.
 *
 * Checks that all markers are lit consistently by computing, per mark, a
 * representative illumination value and then scoring how far each mark
 * deviates from the board-wide mean.
 *
 * Illumination sampling:
 *
 * Tessellated grids (CHESSBOARD, CHARUCO, APRIL_TAG). Nearest-white-cell proxy:
 *   Each cell is eroded with a rectangular kernel and sampled for min, max,
 *   and a mean proxy ((min + max) / 2). Cells are classified as dark or light
 *   using a dynamic threshold set to the midpoint of the global min and max
 *   mean values.
 *   - Light cells: illumination = cell's own peak (maxVal).
 *   - Dark cells: illumination = maxVal of the nearest light cell (Euclidean
 *     centroid distance). This borrows the local white reference so that dark
 *     cells reflect the illumination at their position rather than their own
 *     (inherently darker) pixel values.
 *
 * Isolated blob patterns (SYMMETRIC_CIRCLES, ASYMMETRIC_CIRCLES). Background ring:
 *   Each blob contour is dilated with a circular kernel to create a hollow
 *   ring spanning the marker and its immediate background. The illumination
 *   value is the maximum pixel intensity sampled over that ring region.
 *
 * Scoring:
 *
 * A global mean illumination mu is computed across all marks. Each mark is
 * then scored by how much its illumination deviates from mu:
 *
 *   deviation  = |illuminationVal − mu|
 *   markScore  = clamp(scoreOffsetFactor − deviation / intensityDeviationScale, 0, 1) * 100
 *
 * The board score is derived from the standard deviation of all illumination
 * values (rather than the mean of mark scores), giving a single measure of
 * spread across the whole board:
 *
 *   stdDev     = sqrt(sum((illuminationVal − mu)^2 / N))
 *   boardScore = clamp(scoreOffsetFactor − stdDev / intensityDeviationScale, 0, 1) * 100
 */
class IlluminationCheck : public Plugin<std::shared_ptr<Board>, IlluminationCheckDebug>
{
public: 
    //-----------------------------------------------------------------------------
    // Structs and constants
    //-----------------------------------------------------------------------------

    /** @brief Default values for the plugin parameters */
    struct Defaults {
        static constexpr double kernelSize{ 5.5 };
        static constexpr double intensityDeviationScale{ 40.0 };
        static constexpr double scoreOffsetFactor{ 1.0 };
    };

    static constexpr std::string_view ID{ "illumination" };
    static constexpr std::string_view NAME{ "Illumination Check" };
    static constexpr std::string_view DESCRIPTION{
        R"(Checks for uniform illumination across the board.

Illumination is sampled differently depending on the pattern type:
- CHESSBOARD / CHARUCO: cells are eroded and classified as dark or light using
  the midpoint of the global min/max mean as the threshold. Light cells use
  their own peak intensity; dark cells borrow the peak of their nearest light
  neighbor, giving a position-aware illumination proxy.
- SYMMETRIC_CIRCLES / ASYMMETRIC_CIRCLES: each blob is dilated to include a
  background ring and the maximum intensity over that region is used.

Mark score (clamped to [0, 100]):
  deviation = |illuminationVal - mean(illuminationVals)|
  markScore = clamp(scoreOffsetFactor - deviation / intensityDeviationScale, 0, 1) * 100

Board score (based on standard deviation, not mean of mark scores):
  boardScore = clamp(scoreOffsetFactor - stdDev / intensityDeviationScale, 0, 1) * 100
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

    std::shared_ptr<PluginResult> executeImpl(
        const std::shared_ptr<Board>& board,
        const std::unordered_map<std::string, std::shared_ptr<PluginResult>>& producersResults = {}
    ) const override;

    void validateParameters() const override;

    std::string getMessageForSeverity(EvaluationSeverity severity) const override;

public:
    // Kernel size applied to marks when creating the sampling ROI.
    // Radius in case of circlegrid-like pattern, rectangle size in case of chessboard-like pattern.
    // Valid range: > 0.0
    double kernelSize{ Defaults::kernelSize };

    // Scaling factor used to normalize raw intensity deviations into a score. Valid range: > 0.0
    double intensityDeviationScale{ Defaults::intensityDeviationScale };

    // The maximum possible score before subtractions. Valid range: > 0.0
    double scoreOffsetFactor{ Defaults::scoreOffsetFactor };
};
