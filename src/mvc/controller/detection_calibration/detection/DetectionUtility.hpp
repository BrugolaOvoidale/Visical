#pragma once
#include <queue>
#include <mutex>
#include <mvc/model/detection_calibration/detection/board_sequence/EvaluatedBoardSequence.hpp>
#include <mvc/model/detection_calibration/detection/DetectionResultMap.hpp>
#include "../common/CalibrationStageUtility.hpp"
#include "../common/PluginDebugResultView.hpp"


// Forward declarations
class DetectionResultView;
class ConstrastCheckDebug;
class FocusCheckDebug;
class IlluminationCheckDebug;
class OverexposureCheckDebug;
class FOVCoverageCheckDebug;
class Thread;


/**
 * @brief Handles the processing ("cooking") of detection data into UI-ready views.
 *
 * This class manages the transformation of raw board and sequence results into
 * DetectionResultView objects. It supports asynchronous task tracking,
 * priority-based mailboxes for results, and plugin-specific debug visualization.
 */
class DetectionUtility : public CalibrationStageUtility
{
public:
    //-----------------------------------------------------------------------------
    // Utility classes
    //-----------------------------------------------------------------------------

    /**
     * @brief A container for transporting processed results between threads.
     * 
     * Stored data can be nullptr.
     */
    class Mail
    {
    public:
        /**
         * @brief Constructs a Mail package.
         * @param detResView Processed view data for the UI.
         * @param evalSeq The associated evaluated board sequence.
         */
        Mail(
            const std::shared_ptr<DetectionResultView>& detResView,
			const std::shared_ptr<EvaluatedBoardSequence>& evalSeq)
			: detResView_(detResView), evalSeq_(evalSeq)
        {}

        ~Mail() = default;

        /** @brief Returns the processed view data. */
		const std::shared_ptr<DetectionResultView>& detectionResultView() const { return detResView_; }

        /** @brief Returns the raw sequence data. */
		const std::shared_ptr<EvaluatedBoardSequence>& evaluatedSequence() const { return evalSeq_; }

    private:
        std::shared_ptr<DetectionResultView> detResView_;

		std::shared_ptr<EvaluatedBoardSequence> evalSeq_;
    };


    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    DetectionUtility();

    ~DetectionUtility() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /**
     * @brief Persists detection images to the disk.
     * @param path The directory path where images will be saved.
     * @param results A map of detection IDs to their respective results.
     * @return TaskResult indicating success or failure of the I/O operation.
     */
    static TaskResult saveImages(
        const std::string& path,
        const std::map<DetectionResultMap::Id, std::shared_ptr<DetectionResult>>& results
    );

    /** @brief Processes a single board result for visualization. */
    void cook(const DetectionResultMap::Entry& boardRes);

    /** @brief Processes a full board sequence for visualization. */
    void cook(const std::shared_ptr<EvaluatedBoardSequence>& evalRes);

    /**
     * @brief Generates debug visualization data for a specific plugin.
     * @param pluginId The unique identifier of the plugin.
     * @param evalBoard The board data to analyze.
     * @return An optional view object, empty if this utilty does not
     * have a builder for this plugin.
     */
    std::optional<PluginDebugResultView> buildPluginDebugResult(
        const std::string& pluginId,
        const std::shared_ptr<EvaluatedBoard>& evalBoard
    ) const;

    /**
     * @brief Generates debug visualization data for a specific plugin.
     * @param pluginId The unique identifier of the plugin.
     * @param evalSequence The board-sequence data to analyze.
     * @return An optional view object, empty if this utilty does not
     * have a builder for this plugin.
     */
    std::optional<PluginDebugResultView> buildPluginDebugResult(
        const std::string& pluginId,
        const std::shared_ptr<EvaluatedBoardSequence>& evalSequence
    ) const;

    /**
     * @brief Retrieves a localized tooltip for a specific pixel coordinate.
     * @param pluginId The plugin context for the tooltip logic.
     * @param evalBoard The source data.
     * @param pixel The coordinates within the image.
     */
    std::optional<wxString> getPluginPixelTooltip(
        const std::string& pluginId,
        const std::shared_ptr<EvaluatedBoard>& evalBoard,
        const wxPoint& pixel
    ) const;

    /**
     * @brief Retrieves a localized tooltip for a specific pixel coordinate.
     * @param pluginId The plugin context for the tooltip logic.
     * @param evalSequence The source data.
     * @param pixel The coordinates within the image.
     */
    std::optional<wxString> getPluginPixelTooltip(
        const std::string& pluginId,
        const std::shared_ptr<EvaluatedBoardSequence>& evalSequence,
        const wxPoint& pixel
    ) const;

    /**
     * @brief Retrieves the most recently processed view and clears the slot.
     * @return Latest DetectionResultView, or std::nullopt if none available.
     */
    std::optional<Mail> takeLatest();

    /**
     * @brief Retrieves the next high-priority result from the queue.
     * @return The oldest high-priority result, or std::nullopt if the queue is empty.
     */
    std::optional<Mail> takeHighPriority();

    /** @brief Returns the number of currently active asynchronous cooking tasks. */
    std::uint32_t getRunningCookTasks() const;

    /** @brief Checks if the high-priority mailbox is empty. */
    bool highPriorityEmpty();

private:
    void addCookTask();

    void completeCookTask();

    void storeLatestBoardResult(const DetectionResultMap::Entry& payload);

    void storeLatestSequenceResults(const std::shared_ptr<EvaluatedBoardSequence>& payload);

    void setLatestMail(Mail data);

    void addHighPriorityMail(Mail data);

    PluginDebugResultView buildContrastCheckResult(
        const std::shared_ptr<EvaluatedBoard>& evalBoard,
        const ConstrastCheckDebug& pluginDebRes
    ) const;

    PluginDebugResultView buildFocusCheckResult(
        const std::shared_ptr<EvaluatedBoard>& evalBoard,
        const FocusCheckDebug& pluginDebRes
    ) const;

    PluginDebugResultView buildIlluminationCheckResult(
        const std::shared_ptr<EvaluatedBoard>& evalBoard,
        const IlluminationCheckDebug& pluginDebRes
    ) const;

    PluginDebugResultView buildOverexposureCheckResult(
        const std::shared_ptr<EvaluatedBoard>& evalBoard,
        const OverexposureCheckDebug& pluginDebRes) const;

    PluginDebugResultView buildFOVCoverageCheckResult(
        const std::shared_ptr<EvaluatedBoardSequence>& evalSequence,
        const FOVCoverageCheckDebug& pluginDebRes
    ) const;

    std::optional<wxString> getContrastCheckPixelTooltip(
        const std::shared_ptr<EvaluatedBoard>& evalBoard,
        const ConstrastCheckDebug& pluginDebRes,
        const wxPoint& pixel
    ) const;

    std::optional<wxString> getFocusCheckPixelTooltip(
        const std::shared_ptr<EvaluatedBoard>& evalBoard,
        const FocusCheckDebug& pluginDebRes,
        const wxPoint& pixel
    ) const;

    std::optional<wxString> getIlluminationCheckPixelTooltip(
        const std::shared_ptr<EvaluatedBoard>& evalBoard,
        const IlluminationCheckDebug& pluginDebRes,
        const wxPoint& pixel
    ) const;

    std::optional<wxString> getOverexposureCheckPixelTooltip(
        const OverexposureCheckDebug& pluginDebRes,
        const wxPoint& pixel
    ) const;

private:
    std::unique_ptr<Thread> worker_;

    std::mutex latestMailMutex_;
    std::optional<Mail> latestMail_;

    std::mutex highPriorityMailMutex_;
    std::queue<Mail> highPriorityMail_;

    std::atomic<bool> requestStopProcessBoardRunning_{ false };

    std::atomic<uint32_t> cookTasks_{ 0 };
};
