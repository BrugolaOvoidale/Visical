#include "CalibrationWorkflow.hpp"
#include <mvc/model/detection_calibration/SharedCameraIntrinsics.hpp>
#include <mvc/model/detection_calibration/setup/SetupModel.hpp>
#include <mvc/view/detection_calibration/CalibrationWorkflowView.hpp>
#include <mvc/view/detection_calibration/CalibrationWorkflowViewEvents.hpp>
#include <mvc/view/detection_calibration/setup/SetupPage.hpp>
#include <mvc/view/detection_calibration/setup/SetupPageEvents.hpp>
#include <mvc/view/detection_calibration/detection/DetectionPage.hpp>
#include <mvc/view/detection_calibration/detection/DetectionPageEvents.hpp>
#include <gui_elements/parameter/ParameterWidgetEvents.hpp>
#include "CalibrationWorkflowSettings.hpp"
#include "calibration/CameraIntrinsics.hpp"
#include "detection/DetectionController.hpp"
#include "calibration/CalibrationController.hpp"
#include "CalibrationWorkflowMessages.hpp"
#include "../BaseUtility.hpp"


CalibrationWorkflow::CalibrationWorkflow(
    const std::shared_ptr<CameraManager>& cameraManager,
    const std::shared_ptr<BaseControllerLogger>& logger)
    : BaseController(logger)
{
    init(cameraManager, logger);
}

CalibrationWorkflow::~CalibrationWorkflow() = default;

/////////////////////////////////////////////////////

void CalibrationWorkflow::shutdown()
{
    detController_->shutdown();

    calibController_->shutdown();

    view_->Close(true);
}

void CalibrationWorkflow::notifyCameraConnection(const std::string& cameraId)
{
    detController_->notifyCameraConnection(cameraId);
}

void CalibrationWorkflow::notifyCameraDisconnection(const std::string& cameraId)
{
    detController_->notifyCameraDisconnection(cameraId);
}

/////////////////////////////////////////////////////

void CalibrationWorkflow::init(
    const std::shared_ptr<CameraManager>& cameraManager,
    const std::shared_ptr<BaseControllerLogger>& logger)
{
    if (!cameraManager)
        throw std::invalid_argument("cameraManager is nullptr");
    
    model_ = SetupModel::create();
    model_->freeMode();

    settings_ = std::make_unique<CalibrationWorkflowSettings>();

    view_ = new CalibrationWorkflowView("Visical");

    setupView_ = view_->GetSetupPage();

    DetectionPage* detView = view_->GetDetectionPage();

    calibView_ = view_->GetCalibrationPage();

    // Event bindings
    setupView_->Bind(GUI_SETUP_LOAD_CONFIG, &CalibrationWorkflow::OnLoadSettings, this);
    setupView_->Bind(GUI_SETUP_SAVE_CONFIG, &CalibrationWorkflow::OnSaveSettings, this);
    setupView_->Bind(GUI_SETUP_LOAD_MODEL_PARAMS, &CalibrationWorkflow::OnLoadModelConfigFile, this);
    setupView_->Bind(GUI_SETUP_SAVE_MODEL_PARAMS, &CalibrationWorkflow::OnSaveModelConfigFile, this);
    setupView_->Bind(GUI_SET_PARAM_VALUE, &CalibrationWorkflow::OnEditParam, this);
    setupView_->Bind(GUI_RESET_PARAM, &CalibrationWorkflow::OnResetParam, this);
    setupView_->Bind(GUI_AUTOMATIC_MODE, &CalibrationWorkflow::OnAutomaticMode, this);
    setupView_->Bind(GUI_FROM_HARDWARE_MODE, &CalibrationWorkflow::OnFromHardwareMode, this);
    setupView_->Bind(GUI_FREE_MODE, &CalibrationWorkflow::OnFreeMode, this);

    detView->Bind(GUI_ACCEPT_DATASET, &CalibrationWorkflow::OnAcceptDataset, this);
    detView->Bind(GUI_CAMERA_ASSISTANT, &CalibrationWorkflow::OnOpenCameraAssistant, this);

    view_->Bind(GUI_DEBUG_FRAME, &CalibrationWorkflow::OnShowDebugFrame, this);
    view_->Bind(GUI_CLOSE_APP, &CalibrationWorkflow::OnCloseApp, this);

    detController_ = std::make_unique<DetectionController>(
        detView,
        cameraManager,
        model_->getCamIntrinsics(),
        logger
    );

    calibController_ = std::make_unique<CalibrationController>(
        calibView_,
        model_->getCamIntrinsics(),
        logger
    );

    {
        doLoadSettings();
    }
}

void CalibrationWorkflow::ReloadParameters()
{
    // Setup
    {
        const std::vector<std::shared_ptr<ParameterInfo>> params = model_->getCamIntrinsicsParameters();

        setupView_->SetParameters(params, SetupPage::ParameterLocation::CAMERA_INTRINSICS);

        for (const auto& p : params)
        {
            const std::string& pName = p->name();
            const std::string& pCat = p->category();

            setupView_->MarkParameterAsDirty(pName, pCat, model_->isParameterDirty(pName, pCat));
        }
    }

    // Distortion model
    {
        const std::vector<std::shared_ptr<ParameterInfo>> params = model_->getDistortionModelParameters();

        setupView_->SetParameters(params, SetupPage::ParameterLocation::DISTORTION_MODEL);
    
        for (const auto& p : params)
        {
            const std::string& pName = p->name();
            const std::string& pCat = p->category();

            setupView_->MarkParameterAsDirty(pName, pCat, model_->isParameterDirty(pName, pCat));
        }
    }

    // Camera matrix
    {
        const std::vector<std::shared_ptr<ParameterInfo>> params = model_->getCamMatrixParameters();

        setupView_->SetParameters(params, SetupPage::ParameterLocation::CAMERA_MATRIX);
    
        for (const auto& p : params)
        {
            const std::string& pName = p->name();
            const std::string& pCat = p->category();

            setupView_->MarkParameterAsDirty(pName, pCat, model_->isParameterDirty(pName, pCat));
        }
    }

}

//
void CalibrationWorkflow::CloseApp()
{
    publish(Message(MSG_CLOSE_APP));
}

void CalibrationWorkflow::OnCloseApp(const wxCommandEvent& event)
{
    if(!view_->AskYesNo("Do you want to quit?"))
    {
        return;
    }

    CloseApp();
}

//
void CalibrationWorkflow::doLoadSettings()
{    
    TaskResultP<CalibrationWorkflowSettings::Loaded> res = settings_->loadSettings();

    UpdateLogs(res.takeLogs());

    if (!res.isSuccess())
        return;

    doLoadModelConfigFile(settings_->getModelParamsPath());
    
    switch (res.getPayload().calibMode)
    {
        case CameraIntrinsics::Mode::AUTOMATIC:
            model_->automaticMode();
            setupView_->AutomaticMode();
            break;

        case CameraIntrinsics::Mode::FROM_HARDWARE:
            model_->fromHardwareMode();
            setupView_->FromHardwareMode();
            break;

        case CameraIntrinsics::Mode::FREE:
            model_->freeMode();
            setupView_->FreeMode();
            break;
    }

    ReloadParameters();
}

void CalibrationWorkflow::LoadSettings()
{
    wxString message = "Open .json File";
    wxString wildCard = "JSON files (*.json;*.txt)|*.json;*.txt";

    std::optional<wxString> wxFilePath = view_->OpenFileDialog(message, wildCard);
    if (!wxFilePath.has_value())
        return; // user canceled save dialog


    settings_->setSettingsPath(wxFilePath.value().ToStdString());

    doLoadSettings();
}

void CalibrationWorkflow::OnLoadSettings(const wxCommandEvent& event)
{
    LoadSettings();
}

//
void CalibrationWorkflow::SaveSettings()
{
    wxString message = "Save .json File";
    wxString wildCard = "JSON files (*.json;*.txt)|*.json;*.txt";

    std::optional<wxString> wxFilePath = view_->SaveFileDialog(message, wildCard);
    if (!wxFilePath.has_value())
        return; // user canceled save dialog

    const std::string filePath = wxFilePath.value().ToStdString();

    settings_->setSettingsPath(filePath);

    TaskResult res = settings_->saveSettings(model_->getCamIntrinsics()->get()->mode());

    doSaveModelConfigFile(settings_->getModelParamsPath());

    UpdateLogs(res.takeLogs());
}

void CalibrationWorkflow::OnSaveSettings(const wxCommandEvent& event)
{
    SaveSettings();
}

//
void CalibrationWorkflow::doLoadModelConfigFile(const std::string& filePath)
{
    TaskResultP<std::vector<ParameterOwner::DTO>> res = utils_->loadModelParams(filePath);

    if (res.isSuccess())
    {
        settings_->setModelParamsPath(filePath);

        TaskResult setRes = model_->setParameters(res.takePayload());

        model_->saveParameters();

        ReloadParameters();

        UpdateLogs(setRes.takeLogs());
    }

    UpdateLogs(res.takeLogs());
}

void CalibrationWorkflow::LoadModelConfigFile()
{
    wxString message = "Open .json File";
    wxString wildCard = "JSON files (*.json;*.txt)|*.json;*.txt";

    std::optional<wxString> wxFilePath = view_->OpenFileDialog(message, wildCard);
    if (!wxFilePath.has_value())
        return; // user canceled save dialog

    doLoadModelConfigFile(wxFilePath.value().ToStdString());
}

void CalibrationWorkflow::OnLoadModelConfigFile(const wxCommandEvent& event)
{
    LoadModelConfigFile();
}

//
void CalibrationWorkflow::doSaveModelConfigFile(const std::string& filePath)
{
    TaskResult res = utils_->saveModelParams(filePath, model_->getFilteredParams());

    if (res.isSuccess())
    {
        model_->saveParameters();

        settings_->setModelParamsPath(filePath);

        ReloadParameters();
    }

    UpdateLogs(res.takeLogs());
}

void CalibrationWorkflow::SaveModelConfigFile()
{
    wxString message = "Save .json File";
    wxString wildCard = "JSON files (*.json;*.txt)|*.json;*.txt";

    std::optional<wxString> wxFilePath = view_->SaveFileDialog(message, wildCard);
    if (!wxFilePath.has_value())
        return; // user canceled save dialog

    doSaveModelConfigFile(wxFilePath.value().ToStdString());
}

void CalibrationWorkflow::OnSaveModelConfigFile(const wxCommandEvent& event)
{
    SaveModelConfigFile();
}

//
void CalibrationWorkflow::AcceptDataset()
{
    const bool accepted = calibController_->setWorkingDataset(
        detController_->getDetectedBoards()
    );

    if (accepted)
        view_->GoToPage(wxStaticCast(calibView_, wxWindow));
}

void CalibrationWorkflow::OnAcceptDataset(const wxCommandEvent& event)
{
    AcceptDataset();
}

//
void CalibrationWorkflow::OpenCameraAssistant()
{
    publish(Message(MSG_OPEN_CAMERA_ASSISTANT));
}

void CalibrationWorkflow::OnOpenCameraAssistant(const wxCommandEvent&)
{
    OpenCameraAssistant();
}

//
void CalibrationWorkflow::ShowDebugFrame(const wxCommandEvent& event)
{
    publish(Message(MSG_OPEN_DEBUG_LOGS));
}

void CalibrationWorkflow::OnShowDebugFrame(const wxCommandEvent& event)
{
    ShowDebugFrame(event);
}

//
void CalibrationWorkflow::EditParam(const ParameterChangedEvent& event)
{
    const ParameterChangedEvent::Value& value = event.GetValue();
    const std::string parameterId = event.GetParameterId().ToStdString();
    const std::string categoryId = event.GetCategoryId().ToStdString();

    std::shared_ptr<ParameterInfo> param = model_->getParameter(parameterId, categoryId);
    if (!param) return;

    TaskResult setRes;
    if (auto* stringVal = std::get_if<wxString>(&value))
    {
        setRes = model_->setParameter(
            parameterId,
            categoryId,
            (*stringVal).ToStdString()
        );
    }
    else if (auto* intVal = std::get_if<int>(&value))
    {
        setRes = model_->setParameter(
            parameterId,
            categoryId,
            *intVal
        );
    }
    else if (auto* doubleVal = std::get_if<double>(&value))
    {
        setRes = model_->setParameter(
            parameterId,
            categoryId,
            *doubleVal
        );
    }
    else if (auto* boolVal = std::get_if<bool>(&value))
    {
        setRes = model_->setParameter(
            parameterId,
            categoryId,
            *boolVal
        );
    }
    else
    {
        setRes = model_->executeCommand(
            parameterId,
            categoryId
        );
    }

    if (!setRes.isSuccess())
    {
        setupView_->UpdateParameter(param);
    }
    else
    {
        setupView_->SetParameters(
            model_->getDistortionModelParameters(), SetupPage::ParameterLocation::DISTORTION_MODEL
        );

        setupView_->MarkParameterAsDirty(parameterId, categoryId, model_->isParameterDirty(parameterId, categoryId));
    }
}

void CalibrationWorkflow::OnEditParam(ParameterChangedEvent& event)
{
    EditParam(event);

    event.Skip();
}

//
void CalibrationWorkflow::ResetParam(const ParameterEvent& event)
{
    const std::string parameterId = event.GetParameterId().ToStdString();
    const std::string categoryId = event.GetCategoryId().ToStdString();

    std::shared_ptr<ParameterInfo> param = model_->getParameter(parameterId, categoryId);
    if (!param) return;

    TaskResult resetRes = model_->resetParameter(parameterId, categoryId);

    if (resetRes.isSuccess())
    {
        setupView_->UpdateParameter(param);

        setupView_->SetParameters(
            model_->getDistortionModelParameters(), SetupPage::ParameterLocation::DISTORTION_MODEL
        );

        setupView_->MarkParameterAsDirty(parameterId, categoryId, model_->isParameterDirty(parameterId, categoryId));
    }

    UpdateLogs(resetRes.getLogs());
}

void CalibrationWorkflow::OnResetParam(ParameterEvent& event)
{
    ResetParam(event);
    event.Skip();
}

//
void CalibrationWorkflow::AutomaticMode(const wxCommandEvent& event)
{
    TaskResult res = model_->automaticMode();

    if (res.isSuccess())
    {
        setupView_->SetParameters(
            model_->getCamIntrinsicsParameters(), SetupPage::ParameterLocation::CAMERA_INTRINSICS
        );

        setupView_->SetParameters(
            model_->getDistortionModelParameters(), SetupPage::ParameterLocation::DISTORTION_MODEL
        );
    }

    UpdateLogs(res.getLogs());
}

void CalibrationWorkflow::OnAutomaticMode(const wxCommandEvent& event)
{
    AutomaticMode(event);
}

//
void CalibrationWorkflow::FromHardwareMode(const wxCommandEvent& event)
{
    TaskResult res = model_->fromHardwareMode();

    if (res.isSuccess())
    {
        setupView_->SetParameters(
            model_->getCamIntrinsicsParameters(), SetupPage::ParameterLocation::CAMERA_INTRINSICS
        );

        setupView_->SetParameters(
            model_->getDistortionModelParameters(), SetupPage::ParameterLocation::DISTORTION_MODEL
        );
    }

    UpdateLogs(res.getLogs());
}

void CalibrationWorkflow::OnFromHardwareMode(const wxCommandEvent& event)
{
    FromHardwareMode(event);
}

//
void CalibrationWorkflow::FreeMode(const wxCommandEvent& event)
{
    TaskResult res = model_->freeMode();

    if (res.isSuccess())
    {
        setupView_->SetParameters(
            model_->getCamIntrinsicsParameters(), SetupPage::ParameterLocation::CAMERA_INTRINSICS
        );

        setupView_->SetParameters(
            model_->getDistortionModelParameters(), SetupPage::ParameterLocation::DISTORTION_MODEL
        );

        setupView_->SetParameters(
            model_->getCamMatrixParameters(), SetupPage::ParameterLocation::CAMERA_MATRIX
        );
    }

    UpdateLogs(res.getLogs());
}

void CalibrationWorkflow::OnFreeMode(const wxCommandEvent& event)
{
    FreeMode(event);
}
