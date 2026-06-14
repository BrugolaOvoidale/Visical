#pragma once
#include <evaluator/PluginDebugResult.hpp>
#include <evaluator/EvaluationDefs.hpp>
#include <cv/CvImage.hpp>


/**
 * @brief Container for debug information regarding marks focus.
 */
class FocusCheckDebug : public PluginDebugResult
{
public:
    //-----------------------------------------------------------------------------
    // Structs
    //-----------------------------------------------------------------------------

    /**
     * @brief Focus metrics for an individual detected mark.
     */
    class Mark
    {
    public:
        Mark(
            double sc,
            double meanGrad,
            double normGrad,
            EvaluationSeverity sev)
            : score_(sc),
            meanGradient_(meanGrad),
            normalizedGradient_(normGrad),
            severity_(sev)
        {
        }

        ~Mark() = default;

        double score() const { return score_; }

        double meanGradient() const { return meanGradient_; }

        double normalizedGradient() const { return normalizedGradient_; }

        EvaluationSeverity severity() const { return severity_; }

    private:

        // Focus score in the range [0.0, 100.0]
        double score_;

        // The raw average gradient magnitude across the mark area.
        double meanGradient_;

        // The gradient magnitude adjusted for contrast variations.
        double normalizedGradient_;

        // The severity level assigned to this mark.
        EvaluationSeverity severity_;
    };


    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    /**
     * @brief Constructs the debug report.
	 * @param edgeImage The edge image used for focus analysis.
	 * @param contrast The global contrast metric of the original image.
     * @param marks List of individual mark metrics.
     */
    FocusCheckDebug(
        CvImage edgeImage,
        double contrast,
        std::vector<Mark> marks
    );

	~FocusCheckDebug() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /** @brief Returns the edge image used for focus analysis. */
    const CvImage& edgeImage() const;

    /** @brief Returns the global contrast metric of the original image. */
    double contrast() const;

    /** @brief Returns the list of focus metrics for all analyzed marks. */
    const std::vector<Mark>& marks() const;

private:
    // Stored edge image used by the plugin.
    CvImage edgeImage_;

    // Global contrast value.
    double contrast_;

    // List of per-mark focus data.
    std::vector<Mark> marks_;
};