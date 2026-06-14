#include "AppWorkflow.hpp"
#include <wx/msgdlg.h>
#include <wx/snglinst.h>
#include <wx/image.h>
#include <gui_elements/logger/LoggerView.hpp>
#include <mvc/controller/BaseControllerLogger.hpp>
#include <mvc/controller/camera_assistant/CameraAssistantController.hpp>
#include <mvc/controller/camera_assistant/CameraAssistantControllerMessages.hpp>
#include <mvc/controller/detection_calibration/CalibrationWorkflow.hpp>
#include <mvc/controller/detection_calibration/CalibrationWorkflowMessages.hpp>
#include <camera_manager/CameraManager.hpp>
#include <camera/ICameraMessages.hpp>


AppWorkflow::~AppWorkflow()
{
    if (m_singleInstanceChecker)
    {
        delete m_singleInstanceChecker;
        m_singleInstanceChecker = nullptr;
    }
}

/////////////////////////////////////////////////////////////////////////////

void AppWorkflow::bindCamAssistantMessages()
{
    cameraAssistantController_->subscribe(MSG_CAM_ASSIST_CAMERA_CONNECTED, &AppWorkflow::onCameraConnected, this);
    cameraAssistantController_->subscribe(MSG_CAM_ASSIST_CAMERA_DISCONNECTED, &AppWorkflow::onCameraDisconnected, this);
}

void AppWorkflow::bindDetCalibMessages()
{
    calibWorkflow_->subscribe(MSG_OPEN_CAMERA_ASSISTANT, &AppWorkflow::onOpenCameraAssistant, this);
    calibWorkflow_->subscribe(MSG_OPEN_DEBUG_LOGS, &AppWorkflow::onOpenDebugLogs, this);
    calibWorkflow_->subscribe(MSG_CLOSE_APP, &AppWorkflow::onCloseApp, this);
}

//
bool AppWorkflow::Init()
{
    try
    {
        wxString appName = "Visical-" + wxGetUserId();

        m_singleInstanceChecker = new wxSingleInstanceChecker(appName);

        if (m_singleInstanceChecker->IsAnotherRunning())
        {
            wxMessageBox(
                "Visical is already running!",
                "App already running",
                wxOK | wxICON_INFORMATION
            );

            // Clear and exit
            delete m_singleInstanceChecker;
            m_singleInstanceChecker = nullptr;
            return false;
        }

        // Initialize image handlers
        wxInitAllImageHandlers();

        loggerView_ = new LoggerView(nullptr, wxID_ANY, "Logger");

        logger_ = std::make_unique<BaseControllerLogger>(loggerView_);


        std::shared_ptr<CameraManager> cameraManagerPtr = std::make_shared<CameraManager>();

        // Camera Assistant //

        cameraAssistantController_ = std::make_unique<CameraAssistantController>(cameraManagerPtr, logger_);

        bindCamAssistantMessages();
        //                  //


        // Detection/Calibration //

        calibWorkflow_ = std::make_unique<CalibrationWorkflow>(cameraManagerPtr, logger_);

        bindDetCalibMessages();
        //                  //
    }
    catch (const std::exception& e)
    {
        // Clear and exit
        if (m_singleInstanceChecker)
        {
            delete m_singleInstanceChecker;
            m_singleInstanceChecker = nullptr;
        }

        wxMessageBox(e.what(), "*_* Terminating", wxOK | wxICON_ERROR);

        return false;
    }
    catch (...)
    {
        // Clear and exit
        if (m_singleInstanceChecker)
        {
            delete m_singleInstanceChecker;
            m_singleInstanceChecker = nullptr;
        }

        wxMessageBox("Unknown exception", "*_* Terminating", wxOK | wxICON_ERROR);

        return false;
    }

    return true;
}

bool AppWorkflow::OnInit()
{
    return Init();
}

//
void AppWorkflow::cameraConnected(const Message& message)
{
    calibWorkflow_->notifyCameraConnection(message.context().getAttribute<std::string>(CAM_SERIAL));
}

void AppWorkflow::onCameraConnected(const Message& message)
{
    wxTheApp->CallAfter([this, message]() {
        cameraConnected(message);

        logger_->appendMessage(message);
        });

}

//
void AppWorkflow::cameraDisconnected(const Message& message)
{
    calibWorkflow_->notifyCameraDisconnection(message.context().getAttribute<std::string>(CAM_SERIAL));
}

void AppWorkflow::onCameraDisconnected(const Message& message)
{
    wxTheApp->CallAfter([this, message]() {
        cameraDisconnected(message);

        logger_->appendMessage(message);
        });
}

//
void AppWorkflow::openCameraAssistant()
{
    cameraAssistantController_->OpenView();
}

void AppWorkflow::onOpenCameraAssistant(const Message& message)
{
    wxTheApp->CallAfter([this, message]() {
        openCameraAssistant();

        logger_->appendLogs(message.getLogs());
        });
}

//
void AppWorkflow::openDebugLogs()
{
    loggerView_->Open();
}

void AppWorkflow::onOpenDebugLogs(const Message& message)
{
    wxTheApp->CallAfter([this, message]() {
        openDebugLogs();

        logger_->appendLogs(message.getLogs());
        });
}

void AppWorkflow::closeApp()
{
    cameraAssistantController_->shutdown();

    calibWorkflow_->shutdown();

    loggerView_->Close(true);
}

void AppWorkflow::onCloseApp(const Message& message)
{
    wxTheApp->CallAfter([this, message]() {
        logger_->appendLogs(message.getLogs());
        
        closeApp();
        });
}


wxIMPLEMENT_APP(AppWorkflow); // Just one call to wxIMPLEMENT_APP(MyApp) in the entire code
