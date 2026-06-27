#pragma once
#include "../../BaseController.hpp"
#include <mvc/view/detection_calibration/common/evaluation/EvaluationPanel.hpp>


// Forward declarations
class CalibrationStageModel;
class CalibrationStageView;
class CalibrationStageControllerSettings;
class CalibrationStageUtility;
class wxCommandEvent;
class BoardEvent;
class ParameterChangedEvent;
class ParameterEvent;


/**
 * @brief Abstract base controller for handling a calibration stage logic and UI events.
 */
class CalibrationStageController : public BaseController
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    ~CalibrationStageController() = default;

protected:
    // Protected constructor to enforce derivation.
	// @throws std::invalid_argument if any of the required parameters are nullptr.
    CalibrationStageController(
        std::shared_ptr<CalibrationStageModel> model,
        CalibrationStageView* view,
        std::shared_ptr<CalibrationStageControllerSettings> settings,
        std::shared_ptr<CalibrationStageUtility> utils,
        const std::shared_ptr<BaseControllerLogger>& logger = nullptr
    );

    // Initializes the controller's internal state and UI components.
    // Must be called by derived classes.
    void init();

    // Return the shared pointer of the Model.
    std::shared_ptr<CalibrationStageModel> model() const;

    // Return the pointer of the View.
    CalibrationStageView* view() const;

    // Return the shared pointer of the Settings.
    std::shared_ptr<CalibrationStageControllerSettings> settings() const;

    // Return the shared pointer of the utility layer.
    std::shared_ptr<CalibrationStageUtility> utils() const;

    // Virtual hook to refresh internal parameters from the model.
    virtual void ReloadParameters() = 0;

    /**
     * @brief Registers plugins into the UI panel.
     * @param plugins List of plugin contexts to add.
     * @param where Specifies the panel section.
     */
    void AppendPlugins(
        const std::vector<std::shared_ptr<PluginContextInfo>>& plugins,
        EvaluationPanel::PluginLocation where
    );

    // Updates the UI with the latest results from processed plugins.
    void UpdatePluginResults(const std::vector<std::shared_ptr<PluginResult>>& pluginResults);


    //-----------------------------------------------------------------------------
    // View events
    //-----------------------------------------------------------------------------

    // Load Settings
    virtual void doLoadSettingsImpl() = 0;
    void doLoadSettings();
    void LoadSettings();
    void OnLoadSettings(const wxCommandEvent& event);

    // Save Settings
    virtual void doSaveSettingsImpl() = 0;
    void doSaveSettings();
    void SaveSettings();
    void OnSaveSettings(const wxCommandEvent& event);

    // Draw board
    virtual void DrawBoard(const wxCommandEvent& event) = 0;
    void OnDrawBoard(wxCommandEvent& event);

    // Draw marks
    virtual void DrawMarks(const wxCommandEvent& event) = 0;
    void OnDrawMarks(wxCommandEvent& event);

    // Draw WCS
    virtual void DrawWCS(const wxCommandEvent& event) = 0;
    void OnDrawWCS(wxCommandEvent& event);

    // Remove Board
    virtual void RemoveBoard(const BoardEvent& event) = 0;

    // Remove All Boards
    virtual void RemoveAllBoards() = 0;

    // Load Model config File
    void doLoadModelConfigFile(const std::string& filePath);
    void LoadModelConfigFile();
    void OnLoadModelConfigFile(const wxCommandEvent& event);

    // Save Calib config File
    void doSaveModelConfigFile(const std::string& filePath);
    void SaveModelConfigFile();
    void OnSaveModelConfigFile(const wxCommandEvent& event);

    // Add board plugin
    void BoardAddPlugin(const wxCommandEvent& event);
    void OnBoardAddPlugin(const wxCommandEvent& event);

    // Remove board plugin
    void BoardRemovePlugin(const wxCommandEvent& event);
    void OnBoardRemovePlugin(const wxCommandEvent& event);

    // Load board plugins from file
    void doLoadBoardPlugins(const std::string& filePath);
    void LoadBoardPlugins();
    void OnLoadBoardPlugins(const wxCommandEvent& event);

    // Save board plugins to file
    void doSaveBoardPlugins(const std::string& filePath);
    void SaveBoardPlugins();
    void OnSaveBoardPlugins(const wxCommandEvent& event);

    // Add board sequence plugin
    void BoardSequenceAddPlugin(const wxCommandEvent& event);
    void OnBoardSequenceAddPlugin(const wxCommandEvent& event);

    // Remove board sequence plugin
    void BoardSequenceRemovePlugin(const wxCommandEvent& event);
    void OnBoardSequenceRemovePlugin(const wxCommandEvent& event);

    // Load board sequence plugins from file
    void doLoadBoardSequencePlugins(const std::string& filePath);
    void LoadBoardSequencePlugins();
    void OnLoadBoardSequencePlugins(const wxCommandEvent& event);

    // Save board sequence plugins to file
    void doSaveBoardSequencePlugins(const std::string& filePath);
    void SaveBoardSequencePlugins();
    void OnSaveBoardSequencePlugins(const wxCommandEvent& event);

    // Change plugin threshold
    void ChangePluginThreshold(const wxCommandEvent& event);
    void OnChangePluginThreshold(const wxCommandEvent& event);

    // Change global single board plugin threshold
    void ChangeGlobalBoardThreshold(const wxCommandEvent& event);
    void OnChangeGlobalBoardThreshold(const wxCommandEvent& event);

    // Change global board sequence check threshold
    void ChangeGlobalBoardSeqThreshold(const wxCommandEvent& event);
    void OnChangeGlobalBoardSeqThreshold(const wxCommandEvent& event);

private:
    //-----------------------------------------------------------------------------
    // View events
    //-----------------------------------------------------------------------------

    // Parameter edited
    void EditParam(const ParameterChangedEvent& event);
    void OnEditParam(ParameterChangedEvent& event);

    // Parameter reset
    void ResetParam(const ParameterEvent& event);
    void OnResetParam(ParameterEvent& event);

    // Parameter edited
    void EditPluginParam(const ParameterChangedEvent& event);
    void OnEditPluginParam(ParameterChangedEvent& event);

    // Parameter reset
    void ResetPluginParam(const ParameterEvent& event);
    void OnResetPluginParam(ParameterEvent& event);

	// Select Board
    virtual void SelectBoard(const BoardEvent& event) = 0;
    void OnSelectBoard(BoardEvent& event);

    // Enable/disable plugin
    void EnablePlugin(const wxCommandEvent& event);
    void OnEnablePlugin(wxCommandEvent& event);

private:
	// Shared pointer to the Model.
    std::shared_ptr<CalibrationStageModel> model_;

	// Pointer to the View.
    CalibrationStageView* view_;

	// Shared pointer to the controller settings.
    std::shared_ptr<CalibrationStageControllerSettings> settings_;

	// Shared pointer to the utility layer.
    std::shared_ptr<CalibrationStageUtility> utils_;
};
