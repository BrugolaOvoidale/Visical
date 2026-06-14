#pragma once
#include <wx/app.h>
#include <memory>
#include <string>


// Forward declarations
class wxSingleInstanceChecker;
class Message;
class LoggerView;
class BaseControllerLogger;
class CalibrationWorkflow;
class CameraAssistantController;


/**
 * @brief The central orchestrator/mediator for the application's lifecycle and sub-system communication.
 * 
 * This class acts as the entry point of the application and manages the lifetimes of sub-controllers
 * and core workflows.
 * It binds system-wide messages to specific event handlers, ensuring
 * loose coupling between UI frames and underlying hardware/business logic.
 */
class AppWorkflow : public wxApp
{
public:
	//-----------------------------------------------------------------------------
	// Class operators
	//-----------------------------------------------------------------------------

	AppWorkflow() = default;

	~AppWorkflow();

private:
	void bindCamAssistantMessages();

	void bindDetCalibMessages();


	//-----------------------------------------------------------------------------
	// WxWidgets events
	//-----------------------------------------------------------------------------

	bool Init();
	bool OnInit();


	//-----------------------------------------------------------------------------
	// Controllers messages
	//-----------------------------------------------------------------------------

	// CameraAssistantController

	// Camera connected.
	void cameraConnected(const Message& message);
	void onCameraConnected(const Message& message);

	// Camera disconnected.
	void cameraDisconnected(const Message& message);
	void onCameraDisconnected(const Message& message);


	// CalibrationWorkflowController

	// Request open camera assistant frame.
	void openCameraAssistant();
	void onOpenCameraAssistant(const Message& message);

	// Request open debug logs frame.
	void openDebugLogs();
	void onOpenDebugLogs(const Message& message);

	// Request close app.
	void closeApp();
	void onCloseApp(const Message& message);

private:
	// Raw pointer to the UI logger view.
	LoggerView* loggerView_;

	// Shared logging controller instance.
	std::shared_ptr<BaseControllerLogger> logger_;

	// Lifecycle container for calibration sequences.
	std::unique_ptr<CalibrationWorkflow> calibWorkflow_;

	// Lifecycle container for hardware camera interactions.
	std::unique_ptr<CameraAssistantController> cameraAssistantController_;

	// Tool to detect and prevent duplicate application instances.
	wxSingleInstanceChecker* m_singleInstanceChecker{ nullptr };
};
