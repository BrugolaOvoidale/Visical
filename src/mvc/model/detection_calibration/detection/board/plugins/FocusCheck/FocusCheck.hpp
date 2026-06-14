#pragma once
#include "FocusCheckDebug.hpp"
#include "../BoardPlugin.hpp"


/**
 * @brief Plugin for evaluating image sharpness by analyzing specific board marks.
 *
 * This plugin calculates a focus score based on the gradients of detected marks.
 * 
 * Global contrast:
 * 
 * contrast = (contrast range of all marks) / (number of marks)
 * 
 * Mark score:
 * 
 * score = ( (mark mean gradient) / (contrast) ) * (gradientScalingFactor) - (minScoreOffset)
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
        static constexpr double markDilationRadius{ 5.5 };
        static constexpr double sigma{ 0.7 };
        static constexpr double gradientScalingFactor{ 4.5 };
        static constexpr double gradientPercentileDivisor{ 20.0 };
        static constexpr double minScoreOffset{ 0.25 };
    };

    static constexpr std::string_view ID{ "focus" };
    static constexpr std::string_view NAME{ "Focus Check" };
    static constexpr std::string_view DESCRIPTION{
        R"(Evaluates image sharpness on marks
        
        Global contrast:
        contrast = (contrast range of all marks) / (number of marks)

        Mark score:
        score = ( (mark mean gradient) / (contrast) ) * (gradientScalingFactor) - (minScoreOffset)
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

    std::shared_ptr<PluginResult> executeImpl(const std::shared_ptr<Board>& board) const override;

    void validateParameters() const override;

    std::string getMessageForSeverity(EvaluationSeverity severity) const override;

public:
    // Minimum number of marks required to validate focus. Valid range: > 0
    unsigned int minMarksCount{ Defaults::minMarksCount };

	// Radius for dilating mark masks to include edge gradients. Valid range: > 0.0
    double markDilationRadius{ Defaults::markDilationRadius };

	// Gaussian smoothing factor applied before gradient calculation. Valid range: > 0.0
    double sigma{ Defaults::sigma };

	// Scaling factor applied to the calculated gradient values to compute the final focus score. Valid range: > 0.0
    double gradientScalingFactor{ Defaults::gradientScalingFactor };

	// Divisor used to determine the percentile of gradient values considered for scoring. Valid range: > 0.0
    double gradientPercentileDivisor{ Defaults::gradientPercentileDivisor };

	// Offset added to the final focus score to adjust the threshold. Valid range: >= 0.0
	double minScoreOffset{ Defaults::minScoreOffset };
};
