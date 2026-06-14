#pragma once
#include <mvc/model/detection_calibration/detection/DetectionResultMap.hpp>
#include <wx/image.h>


/**
 * @brief A container for visualizing the detection results of a single board.
 *
 * This class couples the raw detection data with a wxImage formatted
 * for display in the UI.
 */
class DetectionResultView
{
public:
    /**
     * @brief Constructs a view for a single board result.
     * @param boardRes The raw detection data entry.
     * @param boardConvImage The processed wxImage ready for rendering.
     */
    DetectionResultView(
        const DetectionResultMap::Entry& boardRes,
        const wxImage& boardConvImage
    );

    ~DetectionResultView() = default;

    /** @brief Gets the raw detection data entry. */
    const DetectionResultMap::Entry& boardResultEntry() const;

    /** @brief Gets the UI-ready image of the board. */
    const wxImage& wxBoardImage() const;

private:
    // Copy of the detection data.
    DetectionResultMap::Entry boardRes_;

    // The displayable bitmap.
    wxImage boardConvImage_;
};
