#pragma once
#include <vector>
#include <optional>
#include <evaluator/PluginDebugResult.hpp>


/**
 * @brief Container for debug information regarding tilt angles coverage.
 */
class TiltCheckDebug : public PluginDebugResult
{
public:
	//-----------------------------------------------------------------------------
	// Auxiliary classes
	//-----------------------------------------------------------------------------

	/**
	 * @brief Represents the spatial orientation and individual scores of a single board.
	 */
	class BoardPose
	{
	public:
		BoardPose(
			double slantScore,
			double panScore,
			double pitchDeg,
			double yawDeg,
			double rollDeg,
			double slant,
			double pan)
			: slantScore_(slantScore),
			panScore_(panScore),
			pitchDeg_(pitchDeg),
			yawDeg_(yawDeg),
			rollDeg_(rollDeg),
			slant_(slant),
			pan_(pan)
		{
		}

		~BoardPose() = default;

		double slantScore() const { return slantScore_; }

		double panScore() const { return panScore_; }

		double pitchDeg() const { return pitchDeg_; }

		double yawDeg() const { return yawDeg_; }

		double rollDeg() const { return rollDeg_; }

		double slant() const { return slant_; }

		double pan() const { return pan_; }

	private:
		// Per-board slant score after computeScores
		double slantScore_;

		// Per-board pan score after computeScores
		double panScore_;

		// Raw Euler pitch (degrees)
		double pitchDeg_;

		// Raw Euler yaw (degrees)
		double yawDeg_;

		// Raw Euler roll (degrees)
		double rollDeg_;

		// Normalized slant (capped & scaled)
		double slant_;

		// Normalized pan (capped & scaled)
		double pan_;
	};


	//-----------------------------------------------------------------------------
	// Constructor/Destructor
	//-----------------------------------------------------------------------------

	/**
	 * @brief Constructs a TiltCheckDebug result object.
	 * @param entries Vector of optional poses (empty if a board pose was not solved).
	 * @param sortedSlant Sorted Slant scores used as in the check.
	 * @param sortedPan Sorted Pan scores used as in the check.
	 * @param used The number of valid boards actually used in the final calculation.
	 * @param slantSc The final aggregated slant score for the entire check.
	 * @param panSc The final aggregated pan score for the entire check.
	 */
	TiltCheckDebug(
		std::vector<std::optional<BoardPose>> entries,
		std::vector<double> sortedSlant,
		std::vector<double> sortedPan,
		int used,
		double slantSc,
		double panSc);

    ~TiltCheckDebug() = default;


	//-----------------------------------------------------------------------------
	// Class operators
	//-----------------------------------------------------------------------------

	/** @brief Gets the list of poses for all boards (includes nullopt for missing boards). */
	const std::vector<std::optional<BoardPose>>& poses() const;

	/** @brief Gets the collection of slant scores sorted in ascending order. */
	const std::vector<double>& sortedSlantScores() const;

	/** @brief Gets the collection of pan scores sorted in ascending order. */
	const std::vector<double>& sortedPanScores() const;

	/** @brief Returns the count of boards successfully processed. */
	int numUsed() const;

	/** @brief Returns the final aggregated slant score. */
	double slantScore() const;

	/** @brief Returns the final aggregated pan score. */
	double panScore() const;

private:
	// Internal storage for the debug information
	std::vector<std::optional<BoardPose>> boardEntries_;

	// Sorted scores for slant, used in the final check and stored for debugging purposes.
    std::vector<double> sortedSlantScores_;

	// Sorted scores for pan, used in the final check and stored for debugging purposes.
	std::vector<double> sortedPanScores_;

	// Number of  boards that were actually used in the final score calculation.
    int numUsed_;

	// Final aggregated slant score.
    double slantScore_;

	// Final aggregated pan score.
	double panScore_;
};