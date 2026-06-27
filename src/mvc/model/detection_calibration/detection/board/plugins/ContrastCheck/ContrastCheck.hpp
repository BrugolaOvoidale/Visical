#pragma once
#include "ConstrastCheckDebug.hpp"
#include "../BoardPlugin.hpp"


/**
 * @brief Plugin for validating the visual contrast between board markers and background.
 *
 * Calculates per-mark contrast metrics to ensure markers are sufficiently
 * distinguishable from their surroundings. The contrast measure depends on the
 * board pattern type, but all values are on the same [0, 255] pixel-intensity
 * scale so the scoring formula is uniform.
 *
 * Contrast measurement:
 *
 * Tessellated grids (CHESSBOARD, CHARUCO, APRIL_TAG). Cross-cell contrast:
 *   Cells are first classified as dark or light using the median of all cell means
 *   as the split point (robust against outliers). Global dark/light baselines are
 *   then built by taking the trimmed mean (10 %) of the darkest cores of dark cells
 *   and the brightest cores of light cells respectively, discarding outliers.
 *   Each cell's contrast is measured against the opposite baseline:
 *     - Dark cell: contrast = globalAvgLight − cell.minVal
 *     - Light cell: contrast = cell.maxVal − globalAvgDark
 *   Before sampling, every cell is eroded with a rectangular kernel
 *   so that edge-transition pixels do not pollute the measurement.
 *
 * Isolated blob patterns (SYMMETRIC_CIRCLES, ASYMMETRIC_CIRCLES). Intra-mark contrast:
 *   Each circular blob is dilated with a circular kernel so that both the marker
 *   and its immediate background ring are included in the sampling region. The
 *   contrast value is the min-to-max pixel range over that region.
 *
 * Scoring:
 *
 * Each mark's raw contrast value r is mapped to a [0, 100] score:
 *
 *   contrastScore = (r − minDetectableContrast) / (sufficientContrastThr − minDetectableContrast)
 *   markScore     = min(contrastScore, 1.0) * 100
 *
 * The score is clamped at 100 so that marks exceeding sufficientContrastThr do not
 * inflate the board total. The board score is the unweighted mean of all mark scores:
 *
 *   boardScore = sum(markScore / N)
 */
class ContrastCheck : public Plugin<std::shared_ptr<Board>, ConstrastCheckDebug>
{
public:
    //-----------------------------------------------------------------------------
    // Structs and constants
    //-----------------------------------------------------------------------------

    /** @brief Default values for the plugin parameters */
    struct Defaults {
        static constexpr unsigned int minDetectableContrast{ 50 };
        static constexpr unsigned int sufficientContrastThr{ 150 };
        static constexpr double kernelSize{ 5.5 };
    };

    static constexpr std::string_view ID{ "contrast" };
    static constexpr std::string_view NAME{ "Contrast Check" };
    static constexpr std::string_view DESCRIPTION{
        R"(Analyzes contrast between board markers and background.

Contrast is measured differently depending on the pattern type:
- CHESSBOARD / CHARUCO: cross-cell contrast. Each cell is compared against
  global dark/light baselines derived from the trimmed means of the opposite
  cell class. Cells are eroded before sampling to avoid edge transitions.
- SYMMETRIC_CIRCLES / ASYMMETRIC_CIRCLES: intra-mark contrast. Each blob is
  dilated to include its background ring, and contrast is the min-to-max
  pixel range over that region.

Mark score (clamped to [0, 100]):
  score = min((r - minDetectableContrast) / (sufficientContrastThr - minDetectableContrast), 1.0) * 100

Board score (mean of all mark scores):
  boardScore = sum(markScores) / N
)"
    };


    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    ~ContrastCheck() = default;


    //-----------------------------------------------------------------------------
    // Construction from factories
    //-----------------------------------------------------------------------------

    /**
     * @brief Factory method to create a plugin instance.
     * @param threshold Initial score threshold.
     * @return A shared pointer to the newly created plugin.
     */
    static std::shared_ptr<ContrastCheck> create(double threshold = DEFAULT_THRESHOLD);

private:
    // Private constructor to ensure instantiation via factory methods.
    ContrastCheck(double threshold);

    std::shared_ptr<PluginResult> executeImpl(
        const std::shared_ptr<Board>& board,
        const std::unordered_map<std::string, std::shared_ptr<PluginResult>>& producersResults = {}
    ) const override;

    void validateParameters() const override;

    std::string getMessageForSeverity(EvaluationSeverity severity) const override;

public:
    // Minimum contrast value to consider a mark detectable. Valid range: (0, 254]
    unsigned int minDetectableContrast{ Defaults::minDetectableContrast };

    // Threshold above which contrast is considered optimal. Valid range: (0, 255]
    unsigned int sufficientContrastThr{ Defaults::sufficientContrastThr };

    // Kernel size applied to marks when creating the sampling ROI.
    // Radius in case of circlegrid-like pattern, rectangle size in case of chessboard-like pattern.
    // Valid range: > 0.0
    double kernelSize{ Defaults::kernelSize };
};
