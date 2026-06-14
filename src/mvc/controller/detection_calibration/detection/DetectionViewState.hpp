#pragma once
#include <mvc/model/detection_calibration/detection/DetectionResult.hpp>


/**
 * @brief Manages the UI selection state for detection results and live frames.
 *
 * This class tracks which board is currently being rendered in the view and
 * which frame is selected as the "live" reference. It acts as a bridge between
 * the data model and the user interface components.
 */
class DetectionViewState
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    DetectionViewState() = default;

	~DetectionViewState() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /**
     * @brief Clears the current rendered board selection.
     */
    void UnsetCurrentRenderedBoard();

    /**
     * @brief Sets the Id of the board to be rendered in the UI.
     * @param resultId The unique identifier of the detection result.
     */
    void SetCurrentRenderedBoard(DetectionResult::Id resultId);

    /**
     * @brief Gets the Id of the board currently being rendered.
     * @return The Id if a board is selected, otherwise std::nullopt.
     */
    std::optional<DetectionResult::Id> GetCurrentRenderedBoard() const;

    /**
     * @brief Sets the Id of the frame currently marked as "live" or active.
     * @param frameId The unique identifier of the frame.
     */
    void SetCurrentLiveFrame(DetectionResult::Id frameId);

    /**
     * @brief Unsets the current live frame.
     */
    void UnsetCurrentLiveFrame();

    /**
     * @brief Gets the Id of the frame currently marked as live.
     * @return The Id if a frame is selected, otherwise std::nullopt.
     */
    std::optional<DetectionResult::Id> GetCurrentLiveFrame() const;

private:
    /** @brief Id of the board currently displayed in the primary view. */
    std::optional<DetectionResult::Id> m_renderedBoardId;

    /** @brief Id of the frame currently selected for live tracking/updates. */
    std::optional<DetectionResult::Id> m_liveFrameId;
};
