#pragma once
#include "ConstrastCheckDebug.hpp"
#include "../BoardPlugin.hpp"


/**
 * @brief Plugin for validating the visual contrast between board surfaces and markers.
 *
 * This plugin calculates contrast metrics to ensure that markers are sufficiently
 * distinguishable from the background.
 * 
 * Mark score:
 * 
 * score = (average contrast of mark pixels - minDetectableContrast) / (sufficientContrastThr - minDetectableContrast)
 * 
 * Total score:
 * 
 * totalScore = score of all marks / number of marks
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
        static constexpr double markDilationRadius{ 5.5 };
    };

    static constexpr std::string_view ID{ "contrast" };
    static constexpr std::string_view NAME{ "Contrast Check" };
    static constexpr std::string_view DESCRIPTION{
        R"(Analyzes contrast between board and marks"

        Mark score:
        score = (average contrast of mark - minDetectableContrast) / (sufficientContrastThr - minDetectableContrast)
         
        Total score:
        totalScore = score of all marks / number of marks
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

    std::shared_ptr<PluginResult> executeImpl(const std::shared_ptr<Board>& board) const override;

    void validateParameters() const override;

    std::string getMessageForSeverity(EvaluationSeverity severity) const override;

public:
    // Minimum contrast value to consider a mark detectable. Valid range: (0, 254]
    unsigned int minDetectableContrast{ Defaults::minDetectableContrast };

    // Threshold above which contrast is considered optimal. Valid range: (0, 255]
    unsigned int sufficientContrastThr{ Defaults::sufficientContrastThr };

    // Dilation radius applied to marks when creating the sampling ROI. Valid range: > 0.0
    double markDilationRadius{ Defaults::markDilationRadius };
};
