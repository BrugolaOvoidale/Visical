#pragma once
#include <evaluator/PluginDebugResult.hpp>
#include <cv/CvContour.hpp>
#include <evaluator/EvaluationDefs.hpp>


/**
 * @brief Container for debug information regarding marks contrast.
 */
class ConstrastCheckDebug : public PluginDebugResult
{
public:
	//-----------------------------------------------------------------------------
	// Structs
	//-----------------------------------------------------------------------------

	/**
	 * @brief Contrast metrics for an individual detected mark.
	 */
	class Mark
	{
	public:
		Mark(
			double sc,
			double rng ,
			EvaluationSeverity sev)
			: score_(sc),
			range_(rng),
			severity_(sev)
		{
		}

		~Mark() = default;

		double score() const { return score_; }

		double range() const { return range_; }

		EvaluationSeverity severity() const { return severity_; }

	private:
		// Contrast score in the range [0.0, 100.0]
		double score_;

		// The difference between max and min intensity found.
		double range_;

		// The severity level assigned to this mark.
		EvaluationSeverity severity_;
	};


	//-----------------------------------------------------------------------------
	// Constructor/Destructor
	//-----------------------------------------------------------------------------

	/**
	 * @brief Constructs the debug report.
	 * @param dilatedMarks Nested vector of contours representing the dilated marks.
	 * @param marks List of individual mark metrics.
	 */
	ConstrastCheckDebug(
		std::vector<std::vector<CvContour>> dilatedMarks,
		double contrast,
		std::vector<Mark> marks
	);

	~ConstrastCheckDebug() = default;


	//-----------------------------------------------------------------------------
	// Class operators
	//-----------------------------------------------------------------------------

	/** @brief Returns the contours used for visual debugging of the dilated areas. */
	const std::vector<std::vector<CvContour>>& dilatedMarks() const;

	/** @brief Returns the calculated board average contrast. */
	double contrast() const;

	/** @brief Returns the list of scores and severities for each mark. */
	const std::vector<Mark>& marks() const;

private:
	// Geometry for debug visualization.
	std::vector<std::vector<CvContour>> dilatedMarks_;

	// Board average contrast
	double contrast_;

	// Data points for contrast analysis.
	std::vector<Mark> marks_;
};