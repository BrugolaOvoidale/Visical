#include "CameraAssistantController.hpp"
#include <wx/app.h>
#include <UtilityFunctions.hpp>
#include <camera/ICameraMessages.hpp>
#include <camera/ICameraInfo.hpp>
#include <camera_manager/CameraManager.hpp>
#include <message_logger/MessageLogger.hpp>
#include <parameter/ParameterDefs.hpp>
#include <gui_elements/parameter/ParameterWidgetEvents.hpp>
#include <mvc/view/camera_assistant/CameraAssistantViewEvents.hpp>
#include <mvc/view/camera_assistant/camera/CameraWidgetEvents.hpp>
#include "CameraAssistantUtility.hpp"
#include "CameraAssistantControllerSettings.hpp"
#include "../BaseControllerLogger.hpp"


/////////////////////////////////////////////////////

static const wxString CAM_PARAM_CAT_ALL{ "All" };

/////////////////////////////////////////////////////


CameraAssistantController::CameraAssistantController(const std::shared_ptr<BaseControllerLogger>& logger)
	: BaseController(logger)
{
	init(std::make_shared<CameraManager>(), logger);
}

CameraAssistantController::CameraAssistantController(
    const std::shared_ptr<CameraManager>& cameraManager,
    const std::shared_ptr<BaseControllerLogger>& logger)
    : BaseController(logger)
{
    init(cameraManager, logger);
}

CameraAssistantController::~CameraAssistantController() = default;

//////////////////////////////////////////////////////////////////////////

void CameraAssistantController::shutdown()
{
    model_->shutdown();

	view_->Close(true);
}

void CameraAssistantController::OpenView()
{
    view_->Open();
}

//////////////////////////////////////////////////////////////////////////

void CameraAssistantController::init(
    const std::shared_ptr<CameraManager>& cameraManager,
    const std::shared_ptr<BaseControllerLogger>& logger)
{
    model_ = std::make_unique<CameraAssistantModel>(cameraManager, std::static_pointer_cast<MessageLogger>(logger));

    view_ = new CameraAssistantView("Camera Assistant", false);

    settings_ = std::make_unique<CameraAssistantControllerSettings>();


    // View
    std::vector<wxString> choices;

    //
    {
        for (const auto& i : CAM_INTERFACES) choices.push_back(UtilityFunctions::stringFromEnum<CameraInterface>(i).value());

        view_->SetInterfaces(choices);
    }

    //
    {
        choices.clear();

        for (const auto& i : CAM_BEHAVIOURS) choices.push_back(UtilityFunctions::stringFromEnum<CameraAssistantModel::CameraBehaviour>(i).value());

        view_->SetDiscoveryBehaviours(choices);
    }


    // Settings
    {
        doLoadSettings();
    }


    // Model bindings
    model_->subscribe(MSG_CAM_ASSIST_AUTO_DISCOVERY, &CameraAssistantController::onCameraEnumerated, this);
    model_->subscribe(MSG_CAM_ASSIST_DISCOVER_INTERFACE, &CameraAssistantController::onCameraEnumerated, this);
    model_->subscribe(MSG_CAM_ASSIST_LOAD_CAM_FROM_CONFIG, &CameraAssistantController::onCameraLoad, this);
    model_->subscribe(MSG_CAM_ASSIST_CHECK_CAMERA, &CameraAssistantController::onCameraCheck, this);
    model_->subscribe(MSG_CAM_ASSIST_CONNECT_CAMERA_SUCCESS, &CameraAssistantController::onCameraConnectionSuccess, this);
    model_->subscribe(MSG_CAM_ASSIST_CONNECT_CAMERA_FAIL, &CameraAssistantController::onCameraConnectionFail, this);
    model_->subscribe(MSG_CAM_ASSIST_DISCONNECT_CAMERA, &CameraAssistantController::onCameraDisconnection, this);
    model_->subscribe(MSG_CAM_ASSIST_CAM_START_LIVE, &CameraAssistantController::onCameraStartLive, this);
    model_->subscribe(MSG_CAM_ASSIST_CAM_STOP_LIVE, &CameraAssistantController::onCameraStopLive, this);
    model_->subscribe(MSG_CAM_ASSIST_REGISTER_CAM, &CameraAssistantController::onRegisterCamera, this);
    model_->subscribe(MSG_CAM_ASSIST_UNREGISTER_CAM, &CameraAssistantController::onUnregisterCamera, this);


    // View bindings
    view_->Bind(GUI_CAM_ASSIST_LOAD_SETTINGS, &CameraAssistantController::OnLoadSettings, this);
    view_->Bind(GUI_CAM_ASSIST_SAVE_SETTINGS, &CameraAssistantController::OnSaveSettings, this);
    view_->Bind(GUI_CAM_ASSIST_SAVE_SETTINGS_AS, &CameraAssistantController::OnSaveSettingsAs, this);
    view_->Bind(GUI_CHANGE_DISCOVERY_BEHAVIOUR, &CameraAssistantController::OnChangeCamDiscoveryBehaviour, this);
    view_->Bind(GUI_LOAD_CAMERAS_FILE, &CameraAssistantController::OnLoadCamerasFromFile, this);
    view_->Bind(GUI_SAVE_CAMERAS_FILE, &CameraAssistantController::OnSaveCamerasToFile, this);
    view_->Bind(GUI_AUTO_DISCOVERY, &CameraAssistantController::OnAutoDiscovery, this);
    view_->Bind(GUI_DISCOVER_INTERFACE, &CameraAssistantController::OnDiscoverInterface, this);
    view_->Bind(GUI_CHECK_CAMERA, &CameraAssistantController::OnCheckCamera, this);
    view_->Bind(GUI_CONNECT_CAMERA, &CameraAssistantController::OnConnectCamera, this);
    view_->Bind(GUI_DISCONNECT_CAMERA, &CameraAssistantController::OnDisconnectCamera, this);
    view_->Bind(GUI_SELECT_CAMERA, &CameraAssistantController::OnCameraClicked, this);
    view_->Bind(GUI_CHANGE_BEHAVIOUR, &CameraAssistantController::OnChangeCameraBehaviour, this);
    view_->Bind(GUI_CHANGE_CATEGORY, &CameraAssistantController::OnChangeCategory, this);
    view_->Bind(GUI_CHANGE_VISIBILITY, &CameraAssistantController::OnChangeVisibility, this);
    view_->Bind(GUI_CHANGE_PARAMS_PERSISTENCE, &CameraAssistantController::OnChangeParamsPersitence, this);
    view_->Bind(GUI_SET_PARAM_VALUE, &CameraAssistantController::OnSetParameter, this);
    view_->Bind(GUI_RESET_PARAM, &CameraAssistantController::OnResetParameter, this);


    // Open behaviour
    if (view_->GetAutoDiscovery())
    {
        const CameraAssistantView::Source camSrc = view_->GetCameraSource();
        if (camSrc == CameraAssistantView::Source::FILE)
        {
            doLoadCamerasFromFile(settings_->getCamerasConfigPath());
        }
        else if (camSrc == CameraAssistantView::Source::ENUMERATION)
        {
            AutoDiscovery();
        }
    }
}

//
void CameraAssistantController::doLoadSettings()
{
    // Settings
    {
        TaskResult res = settings_->loadSettings();

        UpdateLogs(res.takeLogs());
    }

    //
    {
        TaskResultP<CameraAssistantUtility::Config> loadRes = CameraAssistantUtility::loadCameraAssistantConfig(settings_->getSettingsPath());

        UpdateLogs(loadRes.takeLogs());

        if (loadRes.isSuccess())
        {
            CameraAssistantUtility::Config cfg = loadRes.takePayload();

            // Startup behaviour
            {
                view_->EnableAutoDiscovery(cfg.autoDiscovery);
            }

            // Discovery behaviour
            {
                model_->setDefaultCameraBehaviour(cfg.discoveryBehaviour);

                view_->SelectDiscoveryBehaviour(
                    UtilityFunctions::stringFromEnum<CameraAssistantModel::CameraBehaviour>(cfg.discoveryBehaviour).value()
                );
            }

            // Camera source
            {
                // Camera Source
                view_->SetCameraSource(cfg.cameraSource);
            }
        }
    }
}

void CameraAssistantController::LoadSettings()
{
    wxString message = "Open .json File";
    wxString wildCard = "JSON files (*.json)|*.json";

    const std::optional<wxString> filePath = view_->OpenFileDialog(message, wildCard);
    if (!filePath.has_value())
        return; // user canceled dialog


    settings_->setSettingsPath(filePath.value().ToStdString());

    doLoadSettings();
}

void CameraAssistantController::OnLoadSettings(const wxCommandEvent& event)
{
    LoadSettings();
}

//
void CameraAssistantController::SaveSettings()
{
    TaskResult res = settings_->saveSettings(
        view_->GetAutoDiscovery(),
        UtilityFunctions::stringFromEnum<CameraAssistantModel::CameraBehaviour>(model_->getDefaultCameraBehaviour()).value(),
        UtilityFunctions::stringFromEnum<CameraAssistantView::Source>(view_->GetCameraSource()).value()
    );

    UpdateLogs(res.takeLogs());
}

void CameraAssistantController::OnSaveSettings(const wxCommandEvent& event)
{
    SaveSettings();
}

//
void CameraAssistantController::SaveSettingsAs()
{
    wxString message = "Save .json File";
    wxString wildCard = "JSON files (*.json)|*.json";

    const std::optional<wxString> filePath = view_->SaveFileDialog(message, wildCard);
    if (!filePath.has_value())
        return; // user canceled dialog


    settings_->setSettingsPath(filePath.value().ToStdString());

    SaveSettings();
}

void CameraAssistantController::OnSaveSettingsAs(const wxCommandEvent& event)
{
    SaveSettingsAs();
}

//
void CameraAssistantController::ChangeCamDiscoveryBehaviour(const wxCommandEvent& event)
{
    model_->setDefaultCameraBehaviour(
        UtilityFunctions::enumFromString<CameraAssistantModel::CameraBehaviour>(event.GetString().ToStdString()).value()
    );
}

void CameraAssistantController::OnChangeCamDiscoveryBehaviour(const wxCommandEvent& event)
{
    ChangeCamDiscoveryBehaviour(event);
}

//
void CameraAssistantController::AutoDiscovery()
{
    view_->SetUiState(CameraAssistantView::UiState::ENUMERATION);
    model_->autoDiscovery();
}

void CameraAssistantController::OnAutoDiscovery(const wxCommandEvent& event)
{
    AutoDiscovery();
}

//
void CameraAssistantController::DiscoverInterface(const wxCommandEvent& event)
{
	const wxString& interfaceType = event.GetString();

    if (interfaceType.IsEmpty())
        return;

    view_->SetUiState(CameraAssistantView::UiState::ENUMERATION);

    model_->discoverInterface(
        UtilityFunctions::enumFromString<CameraInterface>(interfaceType.ToStdString()).value()
    );
}

void CameraAssistantController::OnDiscoverInterface(const wxCommandEvent& event)
{
    DiscoverInterface(event);
}

//
void CameraAssistantController::doLoadCamerasFromFile(const std::string& filePath)
{
    TaskResultP<std::vector<CameraAssistantModel::CameraDTO>> loadRes = CameraAssistantUtility::loadCamerasConfig(filePath);

    UpdateLogs(loadRes.takeLogs());

    if (loadRes.isSuccess())
    {
        view_->SetCamerasConfigPath(filePath);

        view_->SetUiState(CameraAssistantView::UiState::ENUMERATION);

        model_->loadCamerasConfig(loadRes.takePayload());
    }
}

void CameraAssistantController::LoadCamerasFromFile()
{
    wxString message = "Open .json File";
    wxString wildCard = "JSON files (*.json)|*.json";

    const std::optional<wxString> filePath = view_->OpenFileDialog(message, wildCard);
    if (!filePath.has_value())
        return; // user canceled dialog

    doLoadCamerasFromFile(filePath.value().ToStdString());
}

void CameraAssistantController::OnLoadCamerasFromFile(const wxCommandEvent& event)
{
    LoadCamerasFromFile();
}

//
void CameraAssistantController::SaveCamerasToFile()
{
    wxString message = "Save .json File";
    wxString wildCard = "JSON files (*.json)|*.json";

    const std::optional<wxString> filePath = view_->SaveFileDialog(message, wildCard);
    if (!filePath.has_value())
        return; // user canceled dialog

    const wxString filePath_value = filePath.value();
    const std::string filePath_str = filePath_value.ToStdString();

    TaskResult saveRes = CameraAssistantUtility::saveCamerasConfig(filePath_str, model_->getAllCameras());

    UpdateLogs(saveRes.takeLogs());
}

void CameraAssistantController::OnSaveCamerasToFile(const wxCommandEvent& event)
{
    SaveCamerasToFile();
}

//
void CameraAssistantController::RenderSelectedCameraParams()
{
    std::optional<wxString> wxCameraId = view_->GetSelectedCamera();
    if (!wxCameraId.has_value())
        return;

    const std::string cameraId = wxCameraId.value().ToStdString();


    std::shared_ptr<ICameraInfo> cam = model_->getCamera(cameraId);
    if (!cam)
    {
        UpdateLogs("There is no camera with id:" + cameraId);
        
        ClearCameraParams();

        return;
    }

    if (!cam->isConnected())
    {
        ClearCameraParams();

        return;
    }
    else if (cam->disconnectionRequested())
    {
        ClearCameraParams();

        return;
    }

    view_->SetUiState(CameraAssistantView::UiState::LOADING_CAMERA_PARAMETERS);

    // Prepare all UI data
    std::vector<wxString> categories;
    categories.push_back(CAM_PARAM_CAT_ALL);
    for (const auto& c : cam->getAllCategories()) categories.push_back(c);

    std::vector<wxString> visibilities;
    for (const auto& v : cam->getAllVisibilityLevels())
        visibilities.push_back(UtilityFunctions::stringFromEnum(v).value());

    std::vector<std::shared_ptr<ParameterInfo>> advParams =
        cam->getFilteredParams(std::nullopt, VisibilityLevel::ADVANCED);

    std::vector<std::shared_ptr<ParameterInfo>> basicParams =
        cam->getFilteredParams(std::nullopt, VisibilityLevel::BASIC);

    view_->Freeze();

    view_->SetParameters(std::move(advParams));

    view_->SetCategories(std::move(categories));
    view_->SelectCategory(CAM_PARAM_CAT_ALL);

    view_->SetVisibilities(std::move(visibilities));
    view_->SelectVisibility(UtilityFunctions::stringFromEnum(VisibilityLevel::BASIC).value());

    view_->SetParamsPersistence(cam->areParametersPersistent());
    
    view_->SetUiState(CameraAssistantView::UiState::IDLE);

    view_->Thaw();
}

void CameraAssistantController::ClearCameraParams()
{
    view_->SetParameters({});
    view_->SetCategories({});
    view_->SetVisibilities({});
    view_->SetParamsPersistence(false);
}

void CameraAssistantController::CameraClicked(const wxCommandEvent& event)
{
	const wxString& cameraId = event.GetString();

    if (cameraId.IsEmpty())
        return;

    // Deselect
    if (view_->GetSelectedCamera() == cameraId)
    {
        ClearCameraParams();

        return;
    }

    view_->SelectCamera(cameraId);

    RenderSelectedCameraParams();
}

void CameraAssistantController::OnCameraClicked(const wxCommandEvent& event)
{
    CameraClicked(event);
}

//
void CameraAssistantController::CheckCamera(const wxCommandEvent& event)
{
    const wxString& wxCameraId = event.GetString();

    if (wxCameraId.IsEmpty())
        return;

    TaskEnqueueResult res = model_->checkCamera(wxCameraId.ToStdString());

    if (res.getStatus() == TaskEnqueueResult::Status::NO_ERRORS)
    {
        view_->SetCameraOperation(wxCameraId, CameraWidget::Operation::CHECKING);
    }

    if (res.hasLog())
        UpdateLogs(res.takeLogs());
}

void CameraAssistantController::OnCheckCamera(const wxCommandEvent& event)
{
    CheckCamera(event);
}

//
void CameraAssistantController::ConnectCamera(const wxCommandEvent& event)
{
    const wxString& wxCameraId = event.GetString();

    if (wxCameraId.IsEmpty())
        return;

    TaskEnqueueResult res = model_->connectCamera(wxCameraId.ToStdString());

    if (res.getStatus() == TaskEnqueueResult::Status::NO_ERRORS)
    {
        view_->SetCameraOperation(wxCameraId, CameraWidget::Operation::CONNECTING);
    }

    if (res.hasLog())
        UpdateLogs(res.takeLogs());
}

void CameraAssistantController::OnConnectCamera(const wxCommandEvent& event)
{
    ConnectCamera(event);
}

//
void CameraAssistantController::DisconnectCamera(const wxCommandEvent& event)
{
    const wxString& wxCameraId = event.GetString();

    if (wxCameraId.IsEmpty())
        return;

    TaskEnqueueResult res = model_->disconnectCamera(wxCameraId.ToStdString());

    if (res.getStatus() == TaskEnqueueResult::Status::NO_ERRORS)
    {
        view_->SetCameraOperation(wxCameraId, CameraWidget::Operation::DISCONNECTING);
    }

    if (res.hasLog())
        UpdateLogs(res.takeLogs());
}

void CameraAssistantController::OnDisconnectCamera(const wxCommandEvent& event)
{
    DisconnectCamera(event);
}

//
void CameraAssistantController::ChangeCameraBehaviour(const wxCommandEvent& event)
{
    std::optional<wxString> wxCameraId = view_->GetSelectedCamera();
    if (!wxCameraId.has_value())
        return;

    const std::string cameraId = wxCameraId.value().ToStdString();

    bool setRes = model_->setCameraBehaviour(
        cameraId,
		UtilityFunctions::enumFromString<CameraAssistantModel::CameraBehaviour>(event.GetString().ToStdString()).value()
    );

    if (!setRes)
        UpdateLogs("Camera Id " + cameraId + " does not exists");
}

void CameraAssistantController::OnChangeCameraBehaviour(const wxCommandEvent& event)
{
    ChangeCameraBehaviour(event);
}

//
void CameraAssistantController::ChangeCategory(const wxCommandEvent& event)
{
    std::optional<wxString> wxCameraId = view_->GetSelectedCamera();
    if (!wxCameraId.has_value())
        return;

    const std::string cameraId = wxCameraId.value().ToStdString();

    std::shared_ptr<ICameraInfo> cam = model_->getCamera(cameraId);
    if (!cam)
    {
        UpdateLogs("There is no camera with id: " + cameraId);

        ClearCameraParams();

        return;
    }

    if (!cam->isConnected())
    {
        ClearCameraParams();

        return;
    }

    const std::string categoryId = event.GetString().ToStdString();

    view_->SetUiState(CameraAssistantView::UiState::LOADING_CAMERA_PARAMETERS);

    view_->Freeze();

    std::vector<std::shared_ptr<ParameterInfo>> params;
    if (categoryId == CAM_PARAM_CAT_ALL)
    {
        params = cam->getFilteredParams(
            std::nullopt,
            UtilityFunctions::enumFromString<VisibilityLevel>(view_->GetSelectedVisibility().ToStdString()).value(),
            true
        );
    }
    else
    {
        params = cam->getFilteredParams(
            categoryId,
            UtilityFunctions::enumFromString<VisibilityLevel>(view_->GetSelectedVisibility().ToStdString()).value(),
            true
        );
    }

    view_->SetParameters(params);

    view_->SetUiState(CameraAssistantView::UiState::IDLE);

    view_->Thaw();
}

void CameraAssistantController::OnChangeCategory(const wxCommandEvent& event)
{
    ChangeCategory(event);
}

//
void CameraAssistantController::ChangeVisibility(const wxCommandEvent& event)
{
    std::optional<wxString> wxCameraId = view_->GetSelectedCamera();
    if (!wxCameraId.has_value())
        return;

    const std::string cameraId = wxCameraId.value().ToStdString();

    const VisibilityLevel visibilityMode = UtilityFunctions::enumFromString<VisibilityLevel>(event.GetString().ToStdString()).value();

    std::shared_ptr<ICameraInfo> cam = model_->getCamera(cameraId);
    if (!cam)
    {
        UpdateLogs("There is no camera with id: " + cameraId);

        ClearCameraParams();

        return;
    }

    if (!cam->isConnected())
    {
        ClearCameraParams();

        return;
    }


    view_->SetUiState(CameraAssistantView::UiState::LOADING_CAMERA_PARAMETERS);

    view_->Freeze();

    std::vector<std::shared_ptr<ParameterInfo>> params;
    const wxString& selCategory = view_->GetSelectedCategory();
    if (selCategory == CAM_PARAM_CAT_ALL)
    {
        params = cam->getFilteredParams(
            std::nullopt,
            visibilityMode,
            true
        );
    }
    else
    {
        params = cam->getFilteredParams(
            selCategory.ToStdString(),
            visibilityMode,
            true
        );
    }
    
    view_->SetParameters(params);

    view_->SetUiState(CameraAssistantView::UiState::IDLE);

    view_->Thaw();
}

void CameraAssistantController::OnChangeVisibility(const wxCommandEvent& event)
{
    ChangeVisibility(event);
}

//
void CameraAssistantController::ChangeParamsPersitence(const wxCommandEvent& event)
{
    std::optional<wxString> wxCameraId = view_->GetSelectedCamera();
    if (!wxCameraId.has_value())
        return;

    TaskResult res = model_->setCameraParamsPersistence(
        wxCameraId.value().ToStdString(),
        static_cast<bool>(event.GetInt())
    );

    UpdateLogs(res.takeLogs());
}

void CameraAssistantController::OnChangeParamsPersitence(const wxCommandEvent& event)
{
    ChangeParamsPersitence(event);
}

//
void CameraAssistantController::SetParameter(const ParameterChangedEvent& event)
{
    std::optional<wxString> wxCameraId = view_->GetSelectedCamera();
    if (!wxCameraId.has_value())
        return;

    const std::string cameraId = wxCameraId.value().ToStdString();
    const std::string parameterId = event.GetParameterId().ToStdString();
    const std::string categoryId = event.GetCategoryId().ToStdString();

    const ParameterChangedEvent::Value value = event.GetValue();

    TaskResult res;
    if (auto* stringVal = std::get_if<wxString>(&value))
    {
        res = model_->setCameraParameter(
            cameraId,
            parameterId,
            categoryId,
            (*stringVal).ToStdString()
        );
    }
    else if (auto* intVal = std::get_if<int>(&value))
    {
        res = model_->setCameraParameter(
            cameraId,
            parameterId,
            categoryId,
            *intVal
        );
    }
    else if (auto* doubleVal = std::get_if<double>(&value))
    {
        res = model_->setCameraParameter(
            cameraId,
            parameterId,
            categoryId,
            *doubleVal
        );
    }
    else if (auto* boolVal = std::get_if<bool>(&value))
    {
        res = model_->setCameraParameter(
            cameraId,
            parameterId,
            categoryId,
            *boolVal
        );
    }
    else
    {
        res = model_->executeCameraCommand(
            cameraId,
            parameterId,
            categoryId
        );
    }

    std::shared_ptr<ICameraInfo> cam = model_->getCamera(cameraId);
    if (cam)
    {
        if (!res.isSuccess())
        {
            view_->UpdateParameter(
                cam->getParameter(parameterId, categoryId)
            );
        }

        view_->MarkParameterAsDirty(parameterId, cam->isParameterDirty(parameterId, categoryId));
    }
}

void CameraAssistantController::OnSetParameter(const ParameterChangedEvent& event)
{
    SetParameter(event);
}

//
void CameraAssistantController::ResetParameter(const ParameterEvent& event)
{
    std::optional<wxString> wxCameraId = view_->GetSelectedCamera();
    if (!wxCameraId.has_value())
        return;

    const std::string cameraId = wxCameraId.value().ToStdString();
    const std::string parameterId = event.GetParameterId().ToStdString();
    const std::string categoryId = event.GetCategoryId().ToStdString();

    TaskResult resetRes = model_->resetCameraParameter(cameraId, parameterId, categoryId);

    if (resetRes.isSuccess())
    {
        std::shared_ptr<ICameraInfo> cam = model_->getCamera(cameraId);
        if (cam)
        {
            view_->UpdateParameter(
                cam->getParameter(parameterId, categoryId)
            );

            view_->MarkParameterAsDirty(parameterId, cam->isParameterDirty(parameterId, categoryId));
        }
    }

    UpdateLogs(resetRes.takeLogs());
}

void CameraAssistantController::OnResetParameter(const ParameterEvent& event)
{
    ResetParameter(event);
}

//
void CameraAssistantController::cameraEnumerated(const MessageTask& message)
{
    switch (message.status())
    {
    case MessageTask::TaskStatus::REJECTED:
    case MessageTask::TaskStatus::PENDING:
    case MessageTask::TaskStatus::STARTED:
        break;

    case MessageTask::TaskStatus::SUCCESS:
    case MessageTask::TaskStatus::FAIL:
        view_->SetUiState(CameraAssistantView::UiState::IDLE);
        break;
    }

    UpdateLogsMessage(message);
}

void CameraAssistantController::onCameraEnumerated(const MessageTask& message)
{
    wxTheApp->CallAfter([this, message]() {
        cameraEnumerated(message);
        });
}

//
void CameraAssistantController::cameraLoad(const MessageTask& message)
{
    switch (message.status())
    {
    case MessageTask::TaskStatus::REJECTED:
    case MessageTask::TaskStatus::PENDING:
    case MessageTask::TaskStatus::STARTED:
        break;

    case MessageTask::TaskStatus::SUCCESS:
    case MessageTask::TaskStatus::FAIL:
        view_->SetUiState(CameraAssistantView::UiState::IDLE);
        break;
    }

    UpdateLogsMessage(message);
}

void CameraAssistantController::onCameraLoad(const MessageTask& message)
{
    wxTheApp->CallAfter([this, message]() {
        cameraLoad(message);
        });
}

//
void CameraAssistantController::cameraCheck(const MessageP<CameraState>& message)
{
    const std::string cameraId = message.context().getAttribute<std::string>(CAM_SERIAL);

    std::shared_ptr<ICameraInfo> camInfo = model_->getCamera(cameraId);
    if (camInfo)
    {
        view_->SetCameraOperation(cameraId, CameraWidget::Operation::NONE);
        view_->UpdateCamera(camInfo);
    }

    UpdateLogsMessage(message);
}

void CameraAssistantController::onCameraCheck(const MessageP<CameraState>& message)
{
    wxTheApp->CallAfter([this, message]() {
        cameraCheck(message);
    });
}

//
void CameraAssistantController::cameraConnectionSuccess(const Message& message)
{
    const std::string cameraId = message.context().getAttribute<std::string>(CAM_SERIAL);

    std::shared_ptr<ICameraInfo> camInfo = model_->getCamera(cameraId);
    if (camInfo)
    {
        view_->SetCameraOperation(cameraId, CameraWidget::Operation::NONE);
        view_->UpdateCamera(camInfo);

        RenderSelectedCameraParams();

        publish(Message(MSG_CAM_ASSIST_CAMERA_CONNECTED, SourceId::none(), { {CAM_SERIAL, cameraId} }));
    }

    UpdateLogsMessage(message);
}

void CameraAssistantController::onCameraConnectionSuccess(const Message& message)
{
    wxTheApp->CallAfter([this, message]() {
        cameraConnectionSuccess(message);
    });
}

//
void CameraAssistantController::cameraConnectionFail(const Message& message)
{
    const std::string cameraId = message.context().getAttribute<std::string>(CAM_SERIAL);

    std::shared_ptr<ICameraInfo> camInfo = model_->getCamera(cameraId);
    if (camInfo)
    {
        view_->SetCameraOperation(cameraId, CameraWidget::Operation::NONE);
        view_->UpdateCamera(camInfo);
    }

    UpdateLogsMessage(message);
}

void CameraAssistantController::onCameraConnectionFail(const Message& message)
{
    wxTheApp->CallAfter([this, message]() {
        cameraConnectionFail(message);
    });
}

//
void CameraAssistantController::cameraDisconnection(const Message& message)
{
    const std::string cameraId = message.context().getAttribute<std::string>(CAM_SERIAL);

    std::shared_ptr<ICameraInfo> camInfo = model_->getCamera(cameraId);
    if (camInfo)
    {
        view_->SetCameraOperation(cameraId, CameraWidget::Operation::NONE);
        view_->UpdateCamera(camInfo);
    }

    UpdateLogsMessage(message);

    publish(Message(MSG_CAM_ASSIST_CAMERA_DISCONNECTED, SourceId::none(), { {CAM_SERIAL, cameraId } }));
}

void CameraAssistantController::onCameraDisconnection(const Message& message)
{
    wxTheApp->CallAfter([this, message]() {
        cameraDisconnection(message);
        });
}

//
void CameraAssistantController::cameraStartLive(const Message& message)
{
    const std::string cameraId = message.context().getAttribute<std::string>(CAM_SERIAL);

    std::shared_ptr<ICameraInfo> camInfo = model_->getCamera(cameraId);
    if (camInfo)
    {
        view_->UpdateCamera(camInfo);
    }

    UpdateLogsMessage(message);
}

void CameraAssistantController::onCameraStartLive(const Message& message)
{
    wxTheApp->CallAfter([this, message]() {
        cameraStartLive(message);
        });
}

//
void CameraAssistantController::cameraStopLive(const Message& message)
{
    const std::string cameraId = message.context().getAttribute<std::string>(CAM_SERIAL);

    std::shared_ptr<ICameraInfo> camInfo = model_->getCamera(cameraId);
    if (camInfo)
    {
        view_->UpdateCamera(camInfo);
    }

    UpdateLogsMessage(message);
}

void CameraAssistantController::onCameraStopLive(const Message& message)
{
    wxTheApp->CallAfter([this, message]() {
        cameraStopLive(message);
        });
}

//
void CameraAssistantController::registerCamera(const Message& message)
{
    const std::string cameraId = message.context().getAttribute<std::string>(CAM_SERIAL);

    std::shared_ptr<ICameraInfo> cam = model_->getCamera(cameraId);
    if (!cam)
    {
        UpdateLogs("There is no camera with id:" + cameraId);
    }
    else
    {
        wxArrayString choices;
		choices.reserve(CAM_BEHAVIOURS.size());

        for (const auto& i : CAM_BEHAVIOURS) choices.push_back(UtilityFunctions::stringFromEnum<CameraAssistantModel::CameraBehaviour>(i).value());

        view_->AppendCamera(cam, choices);
    }

    UpdateLogsMessage(message);
}

void CameraAssistantController::onRegisterCamera(const Message& message)
{
    wxTheApp->CallAfter([this, message]() {
        registerCamera(message);
        });
}

//
void CameraAssistantController::unregisterCamera(const Message& message)
{
    const std::string cameraId = message.context().getAttribute<std::string>(CAM_SERIAL);

    if (view_->GetSelectedCamera() == cameraId)
    {
        ClearCameraParams();

        return;
    }

    view_->RemoveCamera(cameraId);

    UpdateLogsMessage(message);
}

void CameraAssistantController::onUnregisterCamera(const Message& message)
{
    wxTheApp->CallAfter([this, message]() {
        unregisterCamera(message);
        });
}
