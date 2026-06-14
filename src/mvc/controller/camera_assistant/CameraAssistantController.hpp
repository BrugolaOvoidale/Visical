#pragma once
#include <message_bus/MessageDispatcherProxy.hpp>
#include <camera/CameraState.hpp>
#include "CameraAssistantControllerMessages.hpp"
#include "../BaseController.hpp"


// Forward declarations
template<typename T>
class MessageP;

class MessageTask;
class BaseControllerLogger;
class CameraManager;
class CameraAssistantModel;
class CameraAssistantView;
class CameraAssistantControllerSettings;
class wxCommandEvent;
class ParameterChangedEvent;
class ParameterEvent;


/**
 * @brief Orchestrates the interaction between the Camera Assistant UI and the underlying hardware models.
 *
 * It inherits from BaseController for life-cycle management and MessageDispatcherProxy
 * to handle asynchronous communications.
 */
class CameraAssistantController : public BaseController, public MessageDispatcherProxy
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    /**
     * @brief Self-contained constructor.
     * @param logger Optional shared pointer to a logging utility.
     */
    CameraAssistantController(const std::shared_ptr<BaseControllerLogger>& logger = nullptr);

    /**
     * @brief Dependency injection constructor.
     * @param cameraManager Shared pointer to an existing manager for camera hardware.
     * @param logger Optional shared pointer to a logging utility.
     */
    CameraAssistantController(
        const std::shared_ptr<CameraManager>& cameraManager,
        const std::shared_ptr<BaseControllerLogger>& logger = nullptr
    );

    ~CameraAssistantController();
    

    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /** @brief Tells the Model to shutdown.*/
    void shutdown();

    /** @brief Show and restore focus on the GUI. */
    void OpenView();

private:
    // Core initialization logic shared by constructors.
    void init(
        const std::shared_ptr<CameraManager>& cameraManager,
        const std::shared_ptr<BaseControllerLogger>& logger
    );

    // Refreshes the parameter list in the UI for the active camera.
    void RenderSelectedCameraParams();

    // Clear the parameter list in the UI.
    void ClearCameraParams();


    //-----------------------------------------------------------------------------
    // View events
    //-----------------------------------------------------------------------------

    // Load settings file
    void doLoadSettings();
    void LoadSettings();
    void OnLoadSettings(const wxCommandEvent& event);

    // Save settings to file
    void SaveSettings();
    void OnSaveSettings(const wxCommandEvent& event);

    // Save settings to specific file
    void SaveSettingsAs();
    void OnSaveSettingsAs(const wxCommandEvent& event);

    // Changed global camera discovery behaviour
    void ChangeCamDiscoveryBehaviour(const wxCommandEvent& event);
    void OnChangeCamDiscoveryBehaviour(const wxCommandEvent& event);

    // Auto discovery plugged cameras
    void AutoDiscovery();
    void OnAutoDiscovery(const wxCommandEvent& event);

    // Discover plugged cameras by interface
    void DiscoverInterface(const wxCommandEvent& event);
    void OnDiscoverInterface(const wxCommandEvent& event);

    // Load cameras from file
    void doLoadCamerasFromFile(const std::string& filePath);
    void LoadCamerasFromFile();
    void OnLoadCamerasFromFile(const wxCommandEvent& event);

    // Save cameras config to file
    void SaveCamerasToFile();
    void OnSaveCamerasToFile(const wxCommandEvent& event);

    // Catch camera clicked event
    void CameraClicked(const wxCommandEvent& event);
    void OnCameraClicked(const wxCommandEvent& event);

    // Check camera status
    void CheckCamera(const wxCommandEvent& event);
    void OnCheckCamera(const wxCommandEvent& event);

    // Connect camera
    void ConnectCamera(const wxCommandEvent& event);
    void OnConnectCamera(const wxCommandEvent& event);

    // Disconnect camera
    void DisconnectCamera(const wxCommandEvent& event);
    void OnDisconnectCamera(const wxCommandEvent& event);

    // Camera behaviour changed
    void ChangeCameraBehaviour(const wxCommandEvent& event);
    void OnChangeCameraBehaviour(const wxCommandEvent& event);

    // Change camera parameters category
    void ChangeCategory(const wxCommandEvent& event);
    void OnChangeCategory(const wxCommandEvent& event);

    // Change camera parameters visibility mode
    void ChangeVisibility(const wxCommandEvent& event);
    void OnChangeVisibility(const wxCommandEvent& event);

	// Change camera parameters persistence
    void ChangeParamsPersitence(const wxCommandEvent& event);
    void OnChangeParamsPersitence(const wxCommandEvent& event);

    // Set camera parameter
    void SetParameter(const ParameterChangedEvent& event);
    void OnSetParameter(const ParameterChangedEvent& event);

    // Disconnect camera
    void ResetParameter(const ParameterEvent& event);
    void OnResetParameter(const ParameterEvent& event);


    //-----------------------------------------------------------------------------
    // Model messages
    //-----------------------------------------------------------------------------

    // Catch load camera from enumeration
    void cameraEnumerated(const MessageTask& message);
    void onCameraEnumerated(const MessageTask& message);
    
    // Catch load camera from config message
    void cameraLoad(const MessageTask& message);
    void onCameraLoad(const MessageTask& message);

    // Catch check camera message
    void cameraCheck(const MessageP<CameraState>& message);
    void onCameraCheck(const MessageP<CameraState>& message);

    // Catch connect camera success message
    void cameraConnectionSuccess(const Message& message);
    void onCameraConnectionSuccess(const Message& message);

    // Catch connect camera fail message
    void cameraConnectionFail(const Message& message);
    void onCameraConnectionFail(const Message& message);

    // Catch disconnect camera message
    void cameraDisconnection(const Message& message);
    void onCameraDisconnection(const Message& message);

	// Catch start live message
    void cameraStartLive(const Message& message);
    void onCameraStartLive(const Message& message);

	// Catch stop live message
    void cameraStopLive(const Message& message);
    void onCameraStopLive(const Message& message);

    // Catch register camera message
    void registerCamera(const Message& message);
    void onRegisterCamera(const Message& message);

    // Catch unregister camera message
    void unregisterCamera(const Message& message);
    void onUnregisterCamera(const Message& message);

private:
    // Core of the module.
    std::unique_ptr<CameraAssistantModel> model_;

    // wxWidgets managed UI pointer
    CameraAssistantView* view_;

    // Persistent configuration.
    std::unique_ptr<CameraAssistantControllerSettings> settings_;
};
