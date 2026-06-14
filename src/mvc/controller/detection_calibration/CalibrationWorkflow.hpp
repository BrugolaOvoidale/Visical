#pragma once
#include <message_bus/MessageDispatcherProxy.hpp>
#include "../BaseController.hpp"


// Forward declarations
class BaseControllerLogger;
class CameraManager;
class wxCommandEvent;
class wxCloseEvent;
class ParameterEvent;
class ParameterChangedEvent;
class SetupPage;
class CalibrationPage;
class CalibrationWorkflowView;
class DetectionController;
class CalibrationController;
class SetupModel;
class BaseUtility;
class CalibrationWorkflowSettings;


/**
 * @brief Orchestrates the end-to-end camera calibration process.
 *
 * This class acts as the main controller for the calibration UI, coordinating
 * between the SetupModel, various sub-controllers (Detection/Calibration),
 * and the wxWidgets-based view. It implements the MessageDispatcherProxy
 * to participate in the application's global message bus.
 */
class CalibrationWorkflow : public BaseController, public MessageDispatcherProxy
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    /**
     * @brief Initializes the workflow with required services.
     * @param cameraManager Pointer to the service managing physical/virtual cameras.
     * @param logger Optional logging utility for workflow-specific events.
     * @throws std::invalid_argument if cameraManager is nullptr.
     */
    CalibrationWorkflow(
        const std::shared_ptr<CameraManager>& cameraManager,
        const std::shared_ptr<BaseControllerLogger>& logger = nullptr
    );

    ~CalibrationWorkflow();


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /** @brief Gracefuly shutdown internal resources.*/
    void shutdown();

    /**
     * @brief Notifies the workflow that a camera is now available.
     * @param cameraId Unique identifier of the connected camera.
     */
    void notifyCameraConnection(const std::string& cameraId);

    /**
     * @brief Notifies the workflow that a camera has been closed.
     * @param cameraId Unique identifier of the disconnected camera.
     */
    void notifyCameraDisconnection(const std::string& cameraId);

private:
    // Internal initialization routine called by the constructor.
    void init(
        const std::shared_ptr<CameraManager>& cameraManager,
        const std::shared_ptr<BaseControllerLogger>& logger = nullptr
    );

    // Refreshes parameters from the settings/model into the UI views.
    void ReloadParameters();

    //-----------------------------------------------------------------------------
    // View events
    //-----------------------------------------------------------------------------

    // Close app
    void CloseApp();
    void OnCloseApp(const wxCommandEvent& event);

    // Load Settings
    void doLoadSettings();
    void LoadSettings();
    void OnLoadSettings(const wxCommandEvent& event);

    // Save Settings
    void doSaveModelConfigFile(const std::string& filePath);
    void SaveSettings();
    void OnSaveSettings(const wxCommandEvent& event);

    // Load Model config File
    void doLoadModelConfigFile(const std::string& filePath);
    void LoadModelConfigFile();
    void OnLoadModelConfigFile(const wxCommandEvent& event);

    // Save Calib config File
    void SaveModelConfigFile();
    void OnSaveModelConfigFile(const wxCommandEvent& event);

    // Accept current detected boards as calibration dataset
    void AcceptDataset();
    void OnAcceptDataset(const wxCommandEvent&);

    // Open Camera Assistant frame
    void OpenCameraAssistant();
    void OnOpenCameraAssistant(const wxCommandEvent&);

    // Show debug frame
    void ShowDebugFrame(const wxCommandEvent& event);
    void OnShowDebugFrame(const wxCommandEvent& event);

    // Parameter edited
    void EditParam(const ParameterChangedEvent& event);
    void OnEditParam(ParameterChangedEvent& event);

    // Parameter reset
    void ResetParam(const ParameterEvent& event);
    void OnResetParam(ParameterEvent& event);

    // Automatic mode
    void AutomaticMode(const wxCommandEvent& event);
    void OnAutomaticMode(const wxCommandEvent& event);

    // From hardware mode
    void FromHardwareMode(const wxCommandEvent& event);
    void OnFromHardwareMode(const wxCommandEvent& event);

    // Free mode
    void FreeMode(const wxCommandEvent& event);
    void OnFreeMode(const wxCommandEvent& event);

private:
    SetupPage* setupView_;

    CalibrationPage* calibView_;

    CalibrationWorkflowView* view_;

    std::unique_ptr<DetectionController> detController_;

    std::unique_ptr<CalibrationController> calibController_;

    std::shared_ptr<SetupModel> model_;

    std::unique_ptr<BaseUtility> utils_;

    std::unique_ptr<CalibrationWorkflowSettings> settings_;
};
