#pragma once
#include "../common/CalibrationStageController.hpp"
#include <mvc/model/detection_calibration/detection/DetectionResultMap.hpp>
#include <mvc/model/detection_calibration/detection/board_sequence/EvaluatedBoardSequence.hpp>


// Forward declarations
class CameraManager;
class SharedCameraIntrinsics;
class Board;
class PixelEvent;
template<typename T>
class MessageP;
class MessageTask;
class DetectionModel;
class DetectionPage;
class DetectionSettings;
class DetectionUtility;
class DetectionViewState;
class UiTickProxy;


/**
 * @brief Orchestrates the calibration board detection process.
 *
 * This controller manages the lifecycle of board detection (DetectionModel),
 * and the user interface.
 */
class DetectionController : public CalibrationStageController
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    /**
     * @brief Constructor for the DetectionController.
     * @param view Pointer to the base calibration view.
     * @param cameraManager Shared manager for camera hardware abstraction.
     * @param camIntrinsics Shared container for intrinsic camera parameters.
     * @param logger Optional logger.
     */
    DetectionController(
        DetectionPage* view,
		const std::shared_ptr<CameraManager>& cameraManager,
        const std::shared_ptr<SharedCameraIntrinsics>& camIntrinsics,
        const std::shared_ptr<BaseControllerLogger>& logger = nullptr
    );

    ~DetectionController();


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /** @brief Safely clear resources. */
    void shutdown() override;

    /** @brief Retrieves the list of currently detected calibration boards. */
    std::vector<std::shared_ptr<Board>> getDetectedBoards() const;

    /** @brief Notify when a camera becomes available on the system. */
    void notifyCameraConnection(const std::string& cameraId);

    /** @brief Notify when a camera is removed or loses connection. */
    void notifyCameraDisconnection(const std::string& cameraId);

private:
    // Internal initialization logic.
    void init(const std::shared_ptr<CameraManager>& cameraManager);

    // Main UI update loop handler, called on every tick of the UI thread.
    void OnUiTick();

    // Increments the internal counter of active background detection tasks and triggers the countdown.
    void addDetectionTask();

    // Decrements the task counter and checks for completion.
    void completeDetectionTask();

    // Triggers the countdown.
    void requestStartTimer();

    // Checks if the controller can stop the UI throttle thread.
    bool canIdle();

    /**
     * @brief Updates an existing board entry with new detection results.
     * @param id The registry Id of the board.
     * @param boardRes The new detection data to apply.
     */
    void UpdateBoard(
        std::uint32_t id,
        const std::shared_ptr<DetectionResult>& boardRes
    );

    // Overrides base behavior to refresh detection parameters from settings.
    void ReloadParameters() override;


    //-----------------------------------------------------------------------------
    // View events
    //-----------------------------------------------------------------------------

    // Load Settings
    void doLoadSettingsImpl() override;

    // Save Settings
    void doSaveSettingsImpl() override;

	// Select Board
    void SelectBoard(const BoardEvent& event) override;

    // Remove Board
    void RemoveBoard(const BoardEvent& event) override;
    void OnRemoveBoard(BoardEvent& event);

    // Remove All Boards
    void ClearSessionView();
    void RemoveAllBoards() override;
    void OnRemoveAllBoards(wxCommandEvent& event);

    // Save Image
    void SaveImage();
    void OnSaveImage(const wxCommandEvent& event);

    // Snap
    void Snap();
    void OnSnap(const wxCommandEvent& event);

    // Load Image
    void LoadImg();
    void OnLoadImage(const wxCommandEvent& event);

    // Live camera acquisition
    void LiveCamera();
    void OnLiveCamera(const wxCommandEvent& event);

    // Live camera acquisition
    void AutoCapture(const wxCommandEvent& event);
    void OnAutoCapture(const wxCommandEvent& event);

    // Show plugin debug result
    void ShowPerBoardDebugResult(const wxCommandEvent& event);
    void OnShowPerBoardDebugResult(const wxCommandEvent& event);

    // Show plugin debug result
    void ShowPerSequenceDebugResult(const wxCommandEvent& event);
    void OnShowPerSequenceDebugResult(const wxCommandEvent& event);

    // Show pixel tooltip in the image display
    void ShowPixelTooltip(const PixelEvent& event);
    void OnShowPixelTooltip(const PixelEvent& event);

    //
    void DrawBoard(const wxCommandEvent& event) override;

    //
    void DrawMarks(const wxCommandEvent& event) override;

    //
    void DrawWCS(const wxCommandEvent& event) override;


    //-----------------------------------------------------------------------------
    // Model messages
    //-----------------------------------------------------------------------------

    //
    void boardDetectionFromImage(const MessageTask& message);
    void onBoardDetectionFromImage(const MessageTask& message);

    //
    void boardDetectionFromSnap(const MessageTask& message);
    void onBoardDetectionFromSnap(const MessageTask& message);

    //
    void boardFromLive(const MessageTask& message);
    void onBoardFromLive(const MessageTask& message);

    //
    void boardReDetection(const MessageTask& message);
    void onBoardReDetection(const MessageTask& message);

    //
    void boardReEvaluation(const MessageTask& message);
    void onBoardReEvaluation(const MessageTask& message);

    //
    void boardStored(const MessageP<DetectionResultMap::Entry>& message);
    void onBoardStored(const MessageP<DetectionResultMap::Entry>& message);

    //
    void boardUpdate(const MessageP<DetectionResultMap::Entry>& message);
    void onBoardUpdate(const MessageP<DetectionResultMap::Entry>& message);

    //
    void boardSequenceReEvaluated(const MessageP<std::shared_ptr<EvaluatedBoardSequence>>& message);
    void onBoardSequenceReEvaluated(const MessageP<std::shared_ptr<EvaluatedBoardSequence>>& message);

    //
    void onPatternTypeChanged(const Message& message);

    //
    void onSessionCleared(const Message& message);

private:
    // The data source and business logic.
    std::shared_ptr<DetectionModel> model_;

    // The UI panel.
    DetectionPage* view_;

    // Configuration.
    std::shared_ptr<DetectionSettings> settings_;

    // Helper functions for detection.
    std::shared_ptr<DetectionUtility> utils_;

    // State management.
    std::unique_ptr<DetectionViewState> viewState_;

    // Handles periodic UI refreshes when detections are in progress.
    std::unique_ptr<UiTickProxy> uiTickProxy_;

	// Number of detection tasks in progress, used to manage the UI throttle.
    std::atomic<uint32_t> tasks_{ 0 };
};
