#pragma once
#include <evaluator/PluginDebugResult.hpp>
#include <cv/CvRegion.hpp>


/**
 * @brief Container for debug information regarding board overexposure.
 */
class OverexposureCheckDebug : public PluginDebugResult
{
public:
	//-----------------------------------------------------------------------------
	// Constructor/Destructor
	//-----------------------------------------------------------------------------

	/**
	 * @brief Constructs the debug report.
	 * @param boardRegion The detected region of the board/plate.
	 * @param overexpRegion The specific sub-region identified as overexposed.
	 * @param boardArea The area in pixels of the board.
	 * @param areaOverexp The area in pixels of the overexposed portion.
	 */
	OverexposureCheckDebug(
		const CvRegion& boardRegion,
		const CvRegion& overexpRegion,
		double boardArea,
		double areaOverexp
	);

	~OverexposureCheckDebug() = default;


	//-----------------------------------------------------------------------------
	// Class operators
	//-----------------------------------------------------------------------------

	/** @brief Returns the region representing the entire board. */
	const CvRegion& boardRegion() const;

	/** @brief Returns the region representing the overexposed pixels. */
	const CvRegion& overexposedRegion() const;

	/** @brief Gets the total area of the plate. */
	double boardArea() const;

	/** @brief Gets the total area affected by overexposure. */
	double overexposedArea() const;

private:
	// The spatial mask of the board.
	CvRegion boardRegion_;

	// The spatial mask of the overexposure.
	CvRegion overexpRegion_;

	// The area of the board in pixels.
	double areaPlate_;

	// The area of the overexposed region in pixels.
	double areaOverexp_;
};