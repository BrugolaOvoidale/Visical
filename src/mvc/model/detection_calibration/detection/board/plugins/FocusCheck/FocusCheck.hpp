#pragma once
#include "FocusCheckDebug.hpp"
#include "../BoardPlugin.hpp"
#include "../ContrastCheck/ContrastCheck.hpp"


/**
 * @brief Plugin for evaluating image sharpness by analyzing edge strength at board marks.
 *
 * Sharpness is measured as the mean Gaussian derivative gradient at each mark,
 * normalized by the board's average contrast. Normalizing by contrast makes the
 * focus score invariant to overall scene brightness: a blurry bright scene and a
 * sharp dim scene are not confused.
 *
 * Contrast measurement:
 *
 * Board contrast is retrived from the ContrastCheck plugin, making it an hard requirement.
 *
 * If contrast is zero the plugin fails early, as normalization would be undefined.
 *
 * Sharpness measurement:
 *
 * A Gaussian derivative filter (parameterized by sigma) is applied to the grayscale
 * image to produce an edge-strength map. For each mark contour, the mean gradient
 * magnitude is sampled over the contour region (no erosion or dilation, meaning the raw
 * contour boundary is used, since edges live exactly there).
 *
 * Scoring:
 *
 * Each mark's gradient is normalized by the board contrast and mapped to [0, 100]:
 *
 *   raw = meanGradient / contrast
 *   markScore = clamp(raw * gradientScalingFactor − minScoreOffset, 0, 1) * 100
 *
 * The board score is not the mean of mark scores. Instead, scores are sorted and a
 * percentile element is selected, controlled by gradientPercentileDivisor:
 *
 *   index = round(N / gradientPercentileDivisor), clamped to [0, N-1]
 *   boardScore = sortedScores[index]
 *
 * Using a low percentile rather than the mean makes the board score reflect the
 * weakest-focus region of the board, which is more conservative for calibration quality gating.
 */
class FocusCheck : public Plugin<std::shared_ptr<Board>, FocusCheckDebug>
{
public:
    //-----------------------------------------------------------------------------
    // Structs and constants
    //-----------------------------------------------------------------------------

    /** @brief Default values for the plugin parameters */
    struct Defaults {
        static constexpr unsigned int minMarksCount{ 4 };
        static constexpr double kernelSize{ 5.5 };
        static constexpr double sigma{ 0.7 };
        static constexpr double gradientScalingFactor{ 4.5 };
        static constexpr double gradientPercentileDivisor{ 20.0 };
        static constexpr double minScoreOffset{ 0.25 };
    };

    static constexpr std::array<std::string_view, 1> DEPENDENCIES{ ContrastCheck::ID };
    static constexpr std::string_view ID{ "focus" };
    static constexpr std::string_view NAME{ "Focus Check" };
    static constexpr std::string_view DESCRIPTION{
        R"(Evaluates image sharpness by measuring edge strength at board marks,
normalized by board contrast.

Board contrast is retrived from the ContrastCheck plugin, making it an hard requirement.

Sharpness is the mean Gaussian derivative gradient (sigma-parameterized)
sampled over each mark's contour region.

Mark score (clamped to [0, 100]):
  raw = meanGradient / contrast
  markScore = clamp(raw * gradientScalingFactor - minScoreOffset, 0, 1) * 100

Board score (percentile of sorted mark scores, not their mean):
  index = round(N / gradientPercentileDivisor), clamped to [0, N-1]
  boardScore = sortedScores[index]
)"
    };

    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    ~FocusCheck() = default;


    //-----------------------------------------------------------------------------
    // Construction from factories
    //-----------------------------------------------------------------------------

    /**
     * @brief Factory method to create a plugin instance.
     * @param threshold Initial score threshold.
     * @return A shared pointer to the newly created plugin.
     */
    static std::shared_ptr<FocusCheck> create(double threshold = DEFAULT_THRESHOLD);

private:
    // Private constructor to ensure instantiation via factory methods.
    FocusCheck(double threshold);

    std::shared_ptr<PluginResult> executeImpl(
        const std::shared_ptr<Board>& board,
        const std::unordered_map<std::string, std::shared_ptr<PluginResult>>& producersResults = {}
    ) const override;

    void validateParameters() const override;

    std::string getMessageForSeverity(EvaluationSeverity severity) const override;

public:
    // Minimum number of marks required to validate focus. Valid range: > 0
    unsigned int minMarksCount{ Defaults::minMarksCount };

	// Gaussian smoothing factor applied before gradient calculation. Valid range: > 0.0
    double sigma{ Defaults::sigma };

	// Scaling factor applied to the calculated gradient values to compute the final focus score. Valid range: > 0.0
    double gradientScalingFactor{ Defaults::gradientScalingFactor };

	// Divisor used to determine the percentile of gradient values considered for scoring. Valid range: > 0.0
    double gradientPercentileDivisor{ Defaults::gradientPercentileDivisor };

	// Offset added to the final focus score to adjust the threshold. Valid range: >= 0.0
	double minScoreOffset{ Defaults::minScoreOffset };
};
