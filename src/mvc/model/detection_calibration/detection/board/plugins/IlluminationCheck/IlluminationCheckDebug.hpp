#pragma once
#include <evaluator/PluginDebugResult.hpp>
#include <cv/CvContour.hpp>
#include <evaluator/EvaluationDefs.hpp>


/**
 * @brief Container for debug information regarding marks illumination homogeneity.
 */
class IlluminationCheckDebug : public PluginDebugResult
{
public:
	//-----------------------------------------------------------------------------
	// Structs
	//-----------------------------------------------------------------------------

	/**
	 * @brief Illumination metrics for an individual detected mark.
	 */
	struct Mark
	{
	public:
		Mark(
			double sc,
			double maxVal,
			double dev,
			EvaluationSeverity sev)
			: score_(sc),
			maxGrayValue_(maxVal),
			deviation_(dev),
			severity_(sev)
		{
		}

		~Mark() = default;

		double score() const { return score_; }

		double maxGrayValue() const { return maxGrayValue_; }

		double deviation() const { return deviation_; }

		EvaluationSeverity severity() const { return severity_; }

	private:
		// Illumination score in the range [0.0, 100.0].
		double score_;

		// Peak intensity value in the mark.
		double maxGrayValue_;

		// Standard deviation of pixel intensities.
		double deviation_;

		// The severity level assigned to this mark.
		EvaluationSeverity severity_;
	};

	//-----------------------------------------------------------------------------
	// Constructor/Destructor
	//-----------------------------------------------------------------------------

	/**
	 * @brief Constructs the debug report.
	 * @param dilatedMarks Nested vector of contours representing the dilated marks.
	 * @param mean The global average illumination across all marks.
	 * @param stdDev The global standard deviation across all marks.
	 * @param marks List of individual mark metrics.
	 */
	IlluminationCheckDebug(
		std::vector<std::vector<CvContour>> dilatedMarks,
		double mean,
		double stdDev,
		std::vector<Mark> marks
	);

	~IlluminationCheckDebug() = default;


	//-----------------------------------------------------------------------------
	// Class operators
	//-----------------------------------------------------------------------------

	/** @brief Returns the contours used for the dilated search regions. */
	const std::vector<std::vector<CvContour>>& dilatedMarks() const;

	/** @brief Returns the global mean illumination level. */
	double mean() const;

	/** @brief Returns the global standard deviation of illumination. */
	double stdDev() const;

	/** @brief Returns the collection of detailed metrics for each detected mark. */
	const std::vector<Mark>& marks() const;

private:
	// Geometry for debug visualization.
	std::vector<std::vector<CvContour>> dilatedMarks_;

	// Global average intensity.
	double mean_;

	// Global intensity spread.
	double stdDev_;

	// Data points for illumination analysis.
	std::vector<Mark> marks_;
};
