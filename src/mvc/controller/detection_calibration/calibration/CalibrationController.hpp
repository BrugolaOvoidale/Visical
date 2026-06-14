#pragma once
#include <mvc/model/detection_calibration/calibration/BoardMap.hpp>
#include <mvc/model/detection_calibration/calibration/CalibrationResult.hpp>
#include "../common/CalibrationStageController.hpp"


// Forward declarations
class SharedCameraIntrinsics;
class Board;
class EvaluatedCalibratedBoardEntry;
class CalibrationModel;
class CalibrationPage;
class CalibrationSettings;
class CalibrationUtility;
template<typename T>
class MessageTaskP;
template<typename T>
class MessageP;


/**
 * @brief Orchestrates the calibration with detected boards.
 *
 * This controller manages the lifecycle ofthe calibration processes
 * and the user interface.
 */
class CalibrationController : public CalibrationStageController
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    /**
     * @brief Constructor for the CalibrationController.
     * @param view Pointer to the base calibration view.
     * @param camIntrinsics Shared container for intrinsic camera parameters.
     * @param logger Optional logger.
     */
    CalibrationController(
        CalibrationPage* view,
        const std::shared_ptr<SharedCameraIntrinsics>& camIntrinsics,
        const std::shared_ptr<BaseControllerLogger>& logger = nullptr
    );

    ~CalibrationController() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /**
     * @brief Populates the model with a new set of calibration boards.
     * @param boards A vector of shared pointers to detected boards.
     * @return true if the dataset was successfully loaded.
     */
    bool setWorkingDataset(const std::vector<std::shared_ptr<Board>>& boards);

private:
    // Internal initialization logic.
    void init();

    /**
     * @brief Updates an existing board entry with new calibrated board. Append if not present.
     * @param evalCalibBoard The new calibrated board data to apply.
     */
    void UpdateBoard(const EvaluatedCalibratedBoardEntry& evalCalibBoard);

    /**
     * @brief Updates an existing board entry with new detected board. Append if not present.
     * @param board The new detected board data to apply.ì
     */
    void UpdateBoard(const BoardMap::Entry& board);

    /**
     * @brief Updates an existing board entry with new calibrated boards. Append if not present.
     * @param evalCalibBoards The new calibrated boards.
     */
    void UpdateBoards(const std::vector<EvaluatedCalibratedBoardEntry>& evalCalibBoards);

    /**
     * @brief Updates an existing board entry with new detected boards. Append if not present.
     * @param boards The new detected boards.
     */
    void UpdateBoards(const std::map<BoardMap::Id, std::shared_ptr<Board>>& boards);


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

    // Remove All Boards
    void RemoveAllBoards() override;

    //
    void DrawBoard(const wxCommandEvent& event) override;

    //
    void DrawMarks(const wxCommandEvent& event) override;

    //
    void DrawWCS(const wxCommandEvent& event) override;

    //
    void ReloadParameters() override;

    //
    void StartCalibration();
    void OnStartCalibration(const wxCommandEvent& event);

    //
    void SaveCalibrationResult();
    void OnSaveCalibrationResult(const wxCommandEvent& event);

    //
    void Undistort(const wxCommandEvent& event);
    void OnUndistort(const wxCommandEvent& event);


    //-----------------------------------------------------------------------------
    // Model messages
    //-----------------------------------------------------------------------------

    void cameraCalibration(const MessageTaskP<CalibrationResult>& msg);
    void onCameraCalibration(const MessageTaskP<CalibrationResult>& msg);

    void calibrationParametersChanged(const Message& msg);
    void onCalibrationParametersChanged(const Message& msg);

    void calibrationBoardReevaluated(const MessageP<std::vector<EvaluatedCalibratedBoardEntry>>& msg);
    void onCalibrationBoardReevaluated(const MessageP<std::vector<EvaluatedCalibratedBoardEntry>>& msg);

    void camModelReevaluated(const MessageP<EvaluatedCameraModel>& msg);
    void onCamModelReevaluated(const MessageP<EvaluatedCameraModel>& msg);

private:
    // The data source and business logic.
    std::shared_ptr<CalibrationModel> model_;

    // The UI panel.
    CalibrationPage* view_;

    // Configuration.
    std::shared_ptr<CalibrationSettings> settings_;

    // Helper functions for detection.
    std::shared_ptr<CalibrationUtility> utils_;
};
