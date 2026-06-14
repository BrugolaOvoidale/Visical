#pragma once
#include <evaluator/PluginDebugResult.hpp>
#include <cv/CvImage.hpp>


/**
 * @brief Container for debug information regarding Field Of View (FOV) coverage.
 */
class FOVCoverageCheckDebug : public PluginDebugResult
{
public:
	/**
	 * @brief Constructs the debug report.
	 * @param heatmap Visualization of FOV coverage.
	 */
	FOVCoverageCheckDebug(CvImage heatmap)
		: heatmap_(std::move(heatmap))
	{
	}

	~FOVCoverageCheckDebug() = default;

	/**
	 * @brief Retrieves the coverage heatmap.
	 * @return A constant reference to the CvImage containing heatmap data.
	 */
	const CvImage& heatmap() const { return heatmap_; }

private:
	// Visualization of FOV coverage.
	CvImage heatmap_;
};