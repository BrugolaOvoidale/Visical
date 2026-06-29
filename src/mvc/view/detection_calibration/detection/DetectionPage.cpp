#include "DetectionPage.hpp"
#include <wx/notebook.h>
#include <wx/sizer.h>
#include <wx/bmpbuttn.h>
#include <wx/radiobut.h>
#include <wx/stattext.h>
#include <wx/choice.h>
#include <wx/checkbox.h>
#include <wx/animate.h>
#include <gui_elements/image_panel/ImagePanel.hpp>
#include <gui_elements/parameter/ParameterWidgetList.hpp>
#include <gui_elements/parameter/ParameterWidget.hpp>
#include <gui_elements/persistent_tooltip/PersistentToolTip.hpp>
#include <parameter/ParameterInfo.hpp>
#include "image_preprocess/ImagePreprocess.hpp"
#include "DetectionPageEvents.hpp"
#include "../common/board/BoardWidgetList.hpp"
#include "../common/board/BoardWidgetEvents.hpp"


using namespace DetectionView;

static const wxString SELECTED_PAGE{ wxT("• ") };

///////////////////////////////////////////////////////////////////////

DetectionPage::DetectionPage(
    wxWindow* parent,
    wxWindowID winid,
    const wxPoint& pos,
    const wxSize& size,
    long style,
    const wxString& name)
    : CalibrationStageView(parent, winid, pos, size, style, name)
{
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    m_notebook = CreateNotebook();
    m_notebook->Bind(wxEVT_NOTEBOOK_PAGE_CHANGED, &DetectionPage::OnPageChanged, this);

    mainSizer->Add(m_notebook, 1, wxEXPAND | wxALL, 0);

    SetSizer(mainSizer);
    
    SetUiState(UiState::IDLE);

    m_imagePreprocessFrame = new ImagePreprocess(this, "Pre processing");
}

///////////////////////////////////////////////////////////////////////

void DetectionPage::UnselectCamera()
{
    m_devicesList->SetSelection(wxNOT_FOUND);

    UpdateCameraFeedBtns();
}

void DetectionPage::SelectCamera(const wxString& camera)
{
    m_devicesList->SetStringSelection(camera);

    UpdateCameraFeedBtns();
}

wxString DetectionPage::GetSelectedCamera() const
{
    return m_devicesList->GetStringSelection();
}

bool DetectionPage::IsCameraSelected() const
{
    return m_devicesList->GetStringSelection() != wxEmptyString;
}

void DetectionPage::AddCamera(const wxString& cameraId)
{
    m_devicesList->Append(cameraId);
}

void DetectionPage::RemoveCamera(const wxString& cameraId)
{
	int index = m_devicesList->FindString(cameraId);
	if (index != wxNOT_FOUND)
	{
        m_devicesList->Delete(index);
	}

    if (IsCameraSelected())
        PersistentToolTip::SetToolTip(m_devicesList, GetSelectedCamera());
    else
        PersistentToolTip::RemoveToolTip(m_devicesList);

    UpdateCameraFeedBtns();
}

void DetectionPage::RemoveAllCameras()
{
    m_devicesList->Clear();

    PersistentToolTip::RemoveToolTip(m_devicesList);

    UpdateCameraFeedBtns();
}

void DetectionPage::SetParameters(
    const std::vector<std::shared_ptr<ParameterInfo>>& params,
    DetectionView::ParameterLocation where)
{
    switch (where)
    {
        case ParameterLocation::SetupPattern:
            m_patternParamsList->SetParameters(params);

            break;

        case ParameterLocation::SetupGeometry:
            m_geomParamsList->SetParameters(params);

            break;

        case ParameterLocation::SetupDetection:
            m_detParamsList->SetParameters(params);

            break;

        case ParameterLocation::SetupRefine:
            m_refineParamsList->SetParameters(params);

            break;

        case ParameterLocation::PreProcessing:
            m_imagePreprocessFrame->SetParameters(params, ParameterLocation::PreProcessing);

            break;
    }
}

void DetectionPage::UpdateParameter(const std::shared_ptr<ParameterInfo>& param)
{
    m_imagePreprocessFrame->UpdateParameter(param);

    const std::string& paramId = param->name();
    const std::string& categoryId = param->category();

    std::shared_ptr<ParameterWidget> widget = m_patternParamsList->GetWidget(paramId, categoryId);
    if (widget)
        widget->Update(param);

    widget = m_geomParamsList->GetWidget(paramId, categoryId);
    if (widget)
        widget->Update(param);

    widget = m_detParamsList->GetWidget(paramId, categoryId);
    if (widget)
        widget->Update(param);

    widget = m_refineParamsList->GetWidget(paramId, categoryId);
    if (widget)
        widget->Update(param);
}

void DetectionPage::MarkParameterAsDirty(
    const wxString& paramId,
    const wxString& categoryId,
    bool isDirty)
{
    m_imagePreprocessFrame->MarkParameterAsDirty(paramId, categoryId, isDirty);

    std::shared_ptr<ParameterWidget> widget = m_patternParamsList->GetWidget(paramId, categoryId);
    if (widget)
        widget->MarkAsDirty(isDirty);

    widget = m_geomParamsList->GetWidget(paramId, categoryId);
    if (widget)
        widget->MarkAsDirty(isDirty);

    widget = m_detParamsList->GetWidget(paramId, categoryId);
    if (widget)
        widget->MarkAsDirty(isDirty);

    widget = m_refineParamsList->GetWidget(paramId, categoryId);
    if (widget)
        widget->MarkAsDirty(isDirty);
}

void DetectionPage::SetUiState(UiState uiState)
{
    m_uiState = uiState;

    switch (uiState)
    {
        case UiState::IDLE:
            IdleMode();

            break;

        case UiState::BUSY:
            BusyMode();

            break;

        case UiState::START_LIVE:
            StartLiveMode();

            break;

        case UiState::STOP_LIVE:
            StopLiveMode();

            break;

        case UiState::LIVE:
            LiveMode();

            break;

        case UiState::BOARD_FROM_FILE:
            BoardFromFileMode();

            break;

        case UiState::BOARD_UPDATE:
            BoardUpdateMode();

            break;

        case UiState::BOARD_REEVALUATE:
            BoardReevaluateMode();

            break;

        case UiState::SEQUENCE_REEVALUATE:
            SequenceReevaluateMode();

            break;
    }
}

DetectionPage::UiState DetectionPage::GetUiState() const
{
    return m_uiState;
}

void DetectionPage::SetImageSource(ImageSource src)
{
    switch (src)
    {
    case ImageSource::CAMERA:
        ImgAcqAssistantUIMode();

        break;

    case ImageSource::FILE:
        LoadImagesUIMode();

        break;
    }
}

DetectionPage::ImageSource DetectionPage::GetImageSource() const
{
    return m_imgSrc;
}

void DetectionPage::SetAutoCapture(bool checked)
{
    m_autoCaptureCheckBox->SetValue(checked);
}

void DetectionPage::SelectBoard(std::uint32_t id)
{
    CalibrationStageView::SelectBoard(id);

    UpdateRemoveBoardBtn();
}

//////////////////////////////////////////////////////////////////////////////////////

wxNotebook* DetectionPage::CreateNotebook()
{
    wxNotebook* notebook = new wxNotebook(this, wxID_ANY);
    
    wxPanel* setupDetectionTab = CreateSetupDetectionTab(notebook);

    wxPanel* detectionTab = CreateDetectionSubTab(notebook);

    notebook->AddPage(setupDetectionTab, SELECTED_PAGE + "Setup", true);
    notebook->AddPage(detectionTab, "Detection");

    m_currPage = 0;

    return notebook;
}

wxPanel* DetectionPage::CreateSetupDetectionTab(wxWindow* parent)
{
    wxPanel* panel = new wxPanel(parent);

    // Use a vertical BoxSizer to stack the sections
    wxBoxSizer* vbox = new wxBoxSizer(wxVERTICAL);

    // Load and Save
    wxBoxSizer* settingsSizer = new wxBoxSizer(wxHORIZONTAL);

    wxBitmap openBitmap(wxImage("resources/open.png", wxBITMAP_TYPE_PNG).Rescale(18, 18));
    m_loadConfig = new wxBitmapButton(panel, wxID_ANY, openBitmap);
    m_loadConfig->SetToolTip("Load Detection settings");
    m_loadConfig->Bind(wxEVT_BUTTON, &DetectionPage::OnLoadSettings, this);

    settingsSizer->Add(m_loadConfig, 0);


    wxBitmap saveBitmap(wxImage("resources/save.png", wxBITMAP_TYPE_PNG).Rescale(18, 18));
    m_saveConfig = new wxBitmapButton(panel, wxID_ANY, saveBitmap);
    m_saveConfig->SetToolTip("Save Detection settings to file");
    m_saveConfig->Bind(wxEVT_BUTTON, &DetectionPage::OnSaveSettings, this);
    settingsSizer->Add(m_saveConfig, 0);

    vbox->Add(settingsSizer, 0);

    // Image Source Box
    wxStaticBoxSizer* imgSourceBox = new wxStaticBoxSizer(wxVERTICAL, panel, "Image Source");

    wxBoxSizer* imgAcqSizer = new wxBoxSizer(wxHORIZONTAL);

    m_imageFilesRadio = new wxRadioButton(panel, wxID_ANY, "Files", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    m_imageFilesRadio->Bind(wxEVT_RADIOBUTTON, &DetectionPage::OnImageSourceToFile, this);

    m_imgAcqAssistRadio = new wxRadioButton(panel, wxID_ANY, "Camera Acquisition");
    m_imgAcqAssistRadio->Bind(wxEVT_RADIOBUTTON, &DetectionPage::OnImageSourceToCamera, this);


    wxImage imgAcqAssistImage("resources/gear.png", wxBITMAP_TYPE_PNG);
    imgAcqAssistImage.Rescale(18, 18);
    wxBitmap imgAcqAssistIcon(imgAcqAssistImage);
    m_cameraManager = new wxBitmapButton(panel, wxID_ANY, imgAcqAssistIcon);
    m_cameraManager->Bind(wxEVT_BUTTON, &DetectionPage::OnOpenCameraAssistant, this);

    imgSourceBox->Add(m_imageFilesRadio, 0, wxALL, 0);
    imgSourceBox->Add(imgAcqSizer, 0, wxEXPAND | wxALL, 0);

    imgAcqSizer->Add(m_imgAcqAssistRadio, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 0);
    imgAcqSizer->Add(m_cameraManager, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 0);

    vbox->Add(imgSourceBox, 0, wxEXPAND | wxALL, 0);


    // Load and Save Model params
    wxBoxSizer* modelParamsSizer = new wxBoxSizer(wxHORIZONTAL);

    m_loadModelParams = new wxBitmapButton(panel, wxID_ANY, openBitmap);
    m_loadModelParams->SetToolTip("Load Detection parameters");
    m_loadModelParams->Bind(wxEVT_BUTTON, &DetectionPage::OnLoadModelConfig, this);

    modelParamsSizer->Add(m_loadModelParams, 0);


    m_saveModelParams = new wxBitmapButton(panel, wxID_ANY, saveBitmap);
    m_saveModelParams->SetToolTip("Save Detection parameters to file");
    m_saveModelParams->Bind(wxEVT_BUTTON, &DetectionPage::OnSaveModelConfig, this);
    modelParamsSizer->Add(m_saveModelParams, 0);

    vbox->Add(modelParamsSizer, 0);
    

    // Geometry parameters
    wxStaticBoxSizer* patternParamsBox = new wxStaticBoxSizer(wxVERTICAL, panel, "Pattern");

    m_patternParamsList = new ParameterWidgetList(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE);

    m_patternParamsList->SetMinSize(FromDIP(wxSize(280, 100)));

    patternParamsBox->Add(m_patternParamsList, 1, wxEXPAND | wxALL, 0);

    // Geometry parameters
    wxStaticBoxSizer* geomParamsBox = new wxStaticBoxSizer(wxVERTICAL, panel, "Geometry");

    m_geomParamsList = new ParameterWidgetList(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE);

    m_geomParamsList->SetMinSize(FromDIP(wxSize(280, 100)));

    geomParamsBox->Add(m_geomParamsList, 1, wxEXPAND | wxALL, 0);

    // Detection parameters
    wxStaticBoxSizer* detParamsBox = new wxStaticBoxSizer(wxVERTICAL, panel, "Detection");

    m_detParamsList = new ParameterWidgetList(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE);

    detParamsBox->Add(m_detParamsList, 1, wxEXPAND | wxALL, 0);

    m_detParamsList->SetMinSize(FromDIP(wxSize(280, 100)));

    // Refine parameters
    wxStaticBoxSizer* refineParamsBox = new wxStaticBoxSizer(wxVERTICAL, panel, "Refine");

    m_refineParamsList = new ParameterWidgetList(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE);

    m_refineParamsList->SetMinSize(FromDIP(wxSize(280, 100)));

    refineParamsBox->Add(m_refineParamsList, 1, wxEXPAND | wxALL, 0);

    // Add fileBox to the main layout
    vbox->Add(patternParamsBox, 1, wxEXPAND | wxALL, 0);
    vbox->Add(geomParamsBox, 1, wxEXPAND | wxALL, 0);
    vbox->Add(detParamsBox, 1, wxEXPAND | wxALL, 0);
    vbox->Add(refineParamsBox, 1, wxEXPAND | wxALL, 0);

    // Set the sizer to the panel
    panel->SetSizer(vbox);
    return panel;
}

wxPanel* DetectionPage::CreateDetectionSubTab(wxWindow* parent)
{
    wxPanel* panel = new wxPanel(parent);

    wxBoxSizer* vbox = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* mainContentSizer = new wxBoxSizer(wxHORIZONTAL);

    // Left side
    m_imageDisplayPanel = CreateImageDisplayPanel(panel);

    // Right side
    wxBoxSizer* rightSideSizer = new wxBoxSizer(wxVERTICAL);

    wxPanel* detectionPanel = CreateDetectionPanel(panel);
    detectionPanel->SetMinSize(FromDIP(wxSize(280, 50)));
    rightSideSizer->Add(detectionPanel, 1, wxEXPAND);

    m_evaluationPanel = new EvaluationPanel(panel);
    m_evaluationPanel->SetMinSize(FromDIP(wxSize(280, 40)));
    rightSideSizer->Add(m_evaluationPanel, 1, wxEXPAND);

    mainContentSizer->Add(m_imageDisplayPanel, 1, wxEXPAND);
    mainContentSizer->Add(rightSideSizer, 1, wxEXPAND);

    vbox->Add(mainContentSizer, 1, wxEXPAND);

    panel->SetSizer(vbox);

    return panel;
}

wxPanel* DetectionPage::CreateImageDisplayPanel(wxWindow* parent)
{
    wxPanel* panel = new wxPanel(parent);

    wxStaticBoxSizer* imagePanelBox =
        new wxStaticBoxSizer(wxVERTICAL, panel, "Image Display");

    // Device selection
    wxBoxSizer* deviceSizer = new wxBoxSizer(wxHORIZONTAL);

    m_deviceLabel = new wxStaticText(panel, wxID_ANY, "Camera:");
    m_devicesList = new wxChoice(panel, wxID_ANY);
    m_devicesList->Bind(wxEVT_CHOICE, &DetectionPage::OnChangeCamera, this);
    m_devicesList->SetMaxSize(FromDIP(wxSize(400, -1)));

    deviceSizer->Add(m_deviceLabel, 0, wxALIGN_CENTER_VERTICAL);
    deviceSizer->Add(m_devicesList, 1, wxALIGN_CENTER_VERTICAL | wxLEFT, 5);

    imagePanelBox->Add(deviceSizer, 0, wxEXPAND | wxALL, 5);

    // Image panel
    m_imagePanel = new ImagePanel(panel);
    m_imagePanel->SetBackgroundColour(*wxWHITE);
    m_imagePanel->SetMinSize(FromDIP(wxSize(400, 400)));
    m_imagePanel->SetDisplayMode(ImagePanel::FIT_ASPECT_RATIO);

    imagePanelBox->Add(m_imagePanel, 1, wxEXPAND | wxALL, 5);

    // Draw options
    wxBoxSizer* drawSizer = new wxBoxSizer(wxHORIZONTAL);

    m_drawBoard = new wxCheckBox(panel, wxID_ANY, "Draw board");
    m_drawBoard->Bind(wxEVT_CHECKBOX, &DetectionPage::OnDrawBoard, this);

    m_drawMarks = new wxCheckBox(panel, wxID_ANY, "Draw marks");
    m_drawMarks->Bind(wxEVT_CHECKBOX, &DetectionPage::OnDrawMarks, this);

    m_drawWCS = new wxCheckBox(panel, wxID_ANY, "Draw WCS");
    m_drawWCS->Bind(wxEVT_CHECKBOX, &DetectionPage::OnDrawWCS, this);

    drawSizer->Add(m_drawBoard, 0, wxRIGHT, 5);
    drawSizer->Add(m_drawMarks, 0, wxRIGHT, 5);
    drawSizer->Add(m_drawWCS, 0);

    imagePanelBox->Add(drawSizer, 0, wxEXPAND | wxALL, 5);

    panel->SetSizer(imagePanelBox);

    return panel;
}

wxPanel* DetectionPage::CreateDetectionPanel(wxWindow* parent)
{
    wxPanel* panel = new wxPanel(parent);

    // Detection / Calibration Section
    wxStaticBoxSizer* m_calibBox = new wxStaticBoxSizer(wxHORIZONTAL, panel, "Detection");

    // Create Main Vertical Layout
    wxBoxSizer* mainVerticalSizer = new wxBoxSizer(wxVERTICAL);

    // Feed Panel (full width)
    m_feedPanel = CreateFeedPanel(panel);
    mainVerticalSizer->Add(m_feedPanel, 0/*, wxEXPAND | wxALL, 0*/);

    // Bottom Row: Board List + Controls
    wxBoxSizer* bottomVerticalSizer = new wxBoxSizer(wxVERTICAL);

    // Load and Save
    wxBoxSizer* buttonsSizer = new wxBoxSizer(wxHORIZONTAL);

    wxImage openImage("resources/open.png", wxBITMAP_TYPE_PNG);
    openImage.Rescale(18, 18);

    m_loadImgBtn = new wxBitmapButton(panel, wxID_ANY, openImage);
    m_loadImgBtn->SetToolTip("Load images from file");
    m_loadImgBtn->Bind(wxEVT_BUTTON, &DetectionPage::OnLoadImage, this);
    buttonsSizer->Add(m_loadImgBtn, 0);


    wxImage saveImage("resources/save.png", wxBITMAP_TYPE_PNG);
    saveImage.Rescale(18, 18);

    m_saveImgBtn = new wxBitmapButton(panel, wxID_ANY, saveImage);
    m_saveImgBtn->SetToolTip("Save images to file");
    m_saveImgBtn->Bind(wxEVT_BUTTON, &DetectionPage::OnSaveAllBoards, this);
    buttonsSizer->Add(m_saveImgBtn, 0);


    wxImage minusImage("resources/minus.png", wxBITMAP_TYPE_PNG);
    minusImage.Rescale(18, 18);

    m_removeBoard = new wxBitmapButton(panel, wxID_ANY, minusImage);
    m_removeBoard->SetToolTip("Remove selected board");
    m_removeBoard->Bind(wxEVT_BUTTON, &DetectionPage::OnRemoveBoard, this);
    buttonsSizer->Add(m_removeBoard, 0);


    wxImage broomImage("resources/broom.png", wxBITMAP_TYPE_PNG);
    broomImage.Rescale(18, 18);

    m_removeAllBoards = new wxBitmapButton(panel, wxID_ANY, broomImage);
    m_removeAllBoards->SetToolTip("Remove all boards");
    m_removeAllBoards->Bind(wxEVT_BUTTON, &DetectionPage::OnRemoveAllBoards, this);
    buttonsSizer->Add(m_removeAllBoards, 0);


    // Image pre-process
    wxImage preprocessImage("resources/image_preprocess.png", wxBITMAP_TYPE_PNG);
    preprocessImage.Rescale(18, 18);

    m_imgPreProcBtn = new wxBitmapButton(panel, wxID_ANY, preprocessImage);
    m_imgPreProcBtn->SetToolTip("Open image pre-process settings");
    m_imgPreProcBtn->Bind(wxEVT_BUTTON, &DetectionPage::OnImagePreProcess, this);
    buttonsSizer->Add(m_imgPreProcBtn, 0);


    buttonsSizer->AddStretchSpacer(1);


    // Accept dataset
    m_acceptDatasetBtn = new wxButton(panel, wxID_ANY, "Accept dataset");
    m_acceptDatasetBtn->SetToolTip("Accept detected boards for calibration step");
    m_acceptDatasetBtn->Bind(wxEVT_BUTTON, &DetectionPage::OnAcceptDataset, this);
    buttonsSizer->Add(m_acceptDatasetBtn, 0);

    bottomVerticalSizer->Add(buttonsSizer, 0, wxEXPAND);

    // Boards List
    m_boardsList = new BoardWidgetList(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE);
    m_boardsList->Bind(GUI_BOARD_CLICK, &DetectionPage::OnBoardClick, this);

    bottomVerticalSizer->Add(m_boardsList, 1, wxEXPAND | wxALL, 0);


    // Combine into Main Vertical Layout
    mainVerticalSizer->Add(bottomVerticalSizer, 1, wxEXPAND | wxALL, 0);

    m_calibBox->Add(mainVerticalSizer, 1, wxEXPAND | wxALL, 0);

    panel->SetSizer(m_calibBox);

    return panel;
}

wxPanel* DetectionPage::CreateFeedPanel(wxWindow* parent)
{
    wxPanel* panel = new wxPanel(parent);

    //
    wxBoxSizer* feedSizer = new wxBoxSizer(wxHORIZONTAL);

    m_feedIcon = new wxAnimationCtrl(panel, wxID_ANY, wxNullAnimation, wxDefaultPosition, FromDIP(wxSize(24, 24)));
    if (m_feedIcon->LoadFile("resources/gray_dot.gif"))
    {
        m_feedIcon->Stop(); // Stop the animation
        m_feedIcon->SetBackgroundColour(*wxWHITE);
    }
    m_feedIcon->SetToolTip("Current activity");

    m_feedTxt = new wxStaticText(panel, wxID_ANY, "---");

    feedSizer->Add(m_feedIcon, 0, wxALL | wxALIGN_CENTER_VERTICAL, 0);
    feedSizer->AddSpacer(6);
    feedSizer->Add(m_feedTxt, 1, wxALL | wxALIGN_CENTER_VERTICAL, 0);
    //

    //
    wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);

    m_snapBtn = new wxButton(panel, wxID_ANY, "Snap");
    m_snapBtn->Bind(wxEVT_BUTTON, &DetectionPage::OnSnap, this);

    m_liveCheckBox = new wxCheckBox(panel, wxID_ANY, "Live Camera");
    m_liveCheckBox->Bind(wxEVT_CHECKBOX, &DetectionPage::OnLive, this);

    m_autoCaptureCheckBox = new wxCheckBox(panel, wxID_ANY, "Auto capture");
    m_autoCaptureCheckBox->Bind(wxEVT_CHECKBOX, &DetectionPage::OnAutoCapture, this);
    PersistentToolTip::SetToolTip(
        m_autoCaptureCheckBox,
        "Each detected board with no evaluation issues,"
        "will be captured if they will make a score"
        "contribution of at least 0.1 % in a per-sequence evaluation check"
    );

    btnSizer->Add(m_snapBtn, 0, wxALL | wxALIGN_CENTER_VERTICAL, 0);
    btnSizer->Add(m_liveCheckBox, 0, wxALL | wxALIGN_CENTER_VERTICAL, 0);
    btnSizer->Add(m_autoCaptureCheckBox, 0, wxALL | wxALIGN_CENTER_VERTICAL, 0);
    //


    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->Add(feedSizer, 0, wxEXPAND  | wxALL, 0);
    mainSizer->Add(btnSizer, 0, wxALIGN_LEFT | wxALL, 0);
    mainSizer->AddSpacer(6);

    panel->SetSizer(mainSizer);

    return panel;
}

void DetectionPage::DatasetUpdated()
{
    UpdateAcceptDatasetBtn();

    UpdateSaveImagesBtn();

    UpdateRemoveBoardBtn();

    UpdateRemoveAllBoardsBtn();
}

void DetectionPage::UpdateAcceptDatasetBtn()
{
    const bool enable = m_datasetStatus.detected > 0 && m_uiState == UiState::IDLE;

    if (m_acceptDatasetBtn->IsEnabled() != enable)
        m_acceptDatasetBtn->Enable(enable);
}

void DetectionPage::UpdateSaveImagesBtn()
{
    const bool enable = m_datasetStatus.total > 0 && m_uiState == UiState::IDLE;

    if (m_saveImgBtn->IsEnabled() != enable)
        m_saveImgBtn->Enable(enable);
}

void DetectionPage::UpdateRemoveBoardBtn()
{
    const bool enable = m_boardsList->GetSelectedBoardId().has_value() && (m_uiState == UiState::IDLE || m_uiState == UiState::LIVE);

    if (m_removeBoard->IsEnabled() != enable)
        m_removeBoard->Enable(enable);
}

void DetectionPage::UpdateRemoveAllBoardsBtn()
{
    const bool enable = m_datasetStatus.total > 0 && (m_uiState == UiState::IDLE || m_uiState == UiState::LIVE);

    if (m_removeAllBoards->IsEnabled() != enable)
        m_removeAllBoards->Enable(enable);
}

void DetectionPage::UpdateCameraFeedBtns()
{
    const bool enable = IsCameraSelected() && (m_uiState == UiState::IDLE || m_uiState == UiState::LIVE);

    if (m_snapBtn->IsEnabled() != enable)
        m_snapBtn->Enable(enable);

    if (m_liveCheckBox->IsEnabled() != enable)
        m_liveCheckBox->Enable(enable);

    if (m_autoCaptureCheckBox->IsEnabled() != enable)
        m_autoCaptureCheckBox->Enable(enable);
}

void DetectionPage::IdleMode()
{
    m_imageFilesRadio->Enable();

    m_imgAcqAssistRadio->Enable();

    m_devicesList->Enable();

    m_feedTxt->SetLabel("---");

    if (m_feedIcon->LoadFile("resources/gray_dot.gif"))
    {
        m_feedIcon->Stop(); // Stop the animation
        m_feedIcon->SetBackgroundColour(*wxWHITE);
    }

    m_liveCheckBox->SetValue(false);

    m_loadImgBtn->Enable();

    UpdateSaveImagesBtn();

    UpdateRemoveBoardBtn();

    UpdateRemoveAllBoardsBtn();

    m_imgPreProcBtn->Enable();

    UpdateAcceptDatasetBtn();

    m_evaluationPanel->Enable();

    UpdateCameraFeedBtns();
}

void DetectionPage::BusyMode()
{
    m_imageFilesRadio->Disable();

    m_imgAcqAssistRadio->Disable();

    m_devicesList->Disable();

    m_loadImgBtn->Disable();

    UpdateSaveImagesBtn();

    UpdateRemoveBoardBtn();

    UpdateRemoveAllBoardsBtn();

    m_imgPreProcBtn->Disable();

    UpdateAcceptDatasetBtn();

    UpdateCameraFeedBtns();
}

void DetectionPage::StartLiveMode()
{
    BusyMode();

    m_feedTxt->SetLabel("Starting Live...");

    if (m_feedIcon->LoadFile("resources/progress.gif"))
    {
        m_feedIcon->Play(); // Start the animation
        m_feedIcon->SetBackgroundColour(*wxWHITE);
    }

    m_liveCheckBox->SetValue(true);
}

void DetectionPage::StopLiveMode()
{
    BusyMode();

    m_feedTxt->SetLabel("Stopping Live...");

    if (m_feedIcon->LoadFile("resources/progress.gif"))
    {
        m_feedIcon->Play(); // Start the animation
        m_feedIcon->SetBackgroundColour(*wxWHITE);
    }

    m_liveCheckBox->SetValue(true);
}

void DetectionPage::LiveMode()
{
    IdleMode();

    m_devicesList->Disable();

    m_feedTxt->SetLabel("LIVE");

    if (m_feedIcon->LoadFile("resources/recording.gif"))
    {
        m_feedIcon->Play(); // Start the animation
        m_feedIcon->SetBackgroundColour(*wxWHITE);
    }

    m_liveCheckBox->SetValue(true);

    UpdateAcceptDatasetBtn();

    UpdateSaveImagesBtn();

    UpdateRemoveBoardBtn();

    UpdateRemoveAllBoardsBtn();

    UpdateCameraFeedBtns();

    m_feedPanel->Fit();
    m_feedPanel->GetParent()->Layout();
}

void DetectionPage::BoardFromFileMode()
{
    BusyMode();

    m_evaluationPanel->Disable();

    m_feedTxt->SetLabel("Detecting board from file...");

    if (m_feedIcon->LoadFile("resources/progress.gif"))
    {
        m_feedIcon->Play(); // Start the animation
        m_feedIcon->SetBackgroundColour(*wxWHITE);
    }

    UpdateAcceptDatasetBtn();

    m_feedPanel->Fit();
    m_feedPanel->GetParent()->Layout();
}

void DetectionPage::BoardUpdateMode()
{
    m_evaluationPanel->Disable();

    m_feedTxt->SetLabel("Updating board...");

    if (m_feedIcon->LoadFile("resources/progress.gif"))
    {
        m_feedIcon->Play(); // Start the animation
        m_feedIcon->SetBackgroundColour(*wxWHITE);
    }

    UpdateAcceptDatasetBtn();

    m_feedPanel->Fit();
    m_feedPanel->GetParent()->Layout();
}

void DetectionPage::BoardReevaluateMode()
{
    m_evaluationPanel->Disable();

    m_feedTxt->SetLabel("Reevaluating board...");

    if (m_feedIcon->LoadFile("resources/progress.gif"))
    {
        m_feedIcon->Play(); // Start the animation
        m_feedIcon->SetBackgroundColour(*wxWHITE);
    }

    UpdateAcceptDatasetBtn();

    m_feedPanel->Fit();
    m_feedPanel->GetParent()->Layout();
}

void DetectionPage::SequenceReevaluateMode()
{
    m_evaluationPanel->Disable();

    m_feedTxt->SetLabel("Reevaluating sequence...");

	m_feedTxt->Layout(); // Ensure the text is updated immediately

    if (m_feedIcon->LoadFile("resources/progress.gif"))
    {
        m_feedIcon->Play(); // Start the animation
        m_feedIcon->SetBackgroundColour(*wxWHITE);
    }

    UpdateAcceptDatasetBtn();

    m_feedPanel->Fit();
    m_feedPanel->GetParent()->Layout();
}

void DetectionPage::ImgAcqAssistantUIMode()
{
    m_imgSrc = DetectionPage::ImageSource::CAMERA;

    // Feed Panel
    m_snapBtn->Show();

    m_liveCheckBox->Show();

    m_autoCaptureCheckBox->Show();

    m_loadImgBtn->Hide();
    //

    m_deviceLabel->Show();
    m_devicesList->Show();

    m_feedPanel->GetParent()->Layout();

    m_imageDisplayPanel->Fit();
    m_imageDisplayPanel->GetParent()->Layout();
}

void DetectionPage::LoadImagesUIMode()
{
    m_imgSrc = DetectionPage::ImageSource::FILE;

    // Feed Panel
    m_snapBtn->Hide();

    m_liveCheckBox->Hide();

    m_autoCaptureCheckBox->Hide();

    m_loadImgBtn->Show();
    //

    m_deviceLabel->Hide();
    m_devicesList->Hide();

    m_feedPanel->GetParent()->Layout();
    
    m_imageDisplayPanel->Fit();
    m_imageDisplayPanel->GetParent()->Layout();
}

void DetectionPage::OnBoardClick(BoardEvent& event)
{
    UpdateRemoveBoardBtn();

    event.Skip();
}

void DetectionPage::OnRemoveBoard(wxCommandEvent& event)
{
    const std::optional<std::uint32_t> selectedBoard = m_boardsList->GetSelectedBoardId();
    if (selectedBoard.has_value())
    {
        // Let the parent know this board was clicked
        BoardEvent evt(GUI_DET_REMOVE_BOARD, GetId());
        evt.SetBoardId(selectedBoard.value());

        // Send it to parent
        ProcessEvent(evt);         // sends to this and upwards
    }
}

void DetectionPage::OnRemoveAllBoards(wxCommandEvent& event)
{
    // Let the parent know this board was clicked
    wxCommandEvent evt(GUI_DET_REMOVE_ALL_BOARDS, GetId());

    // Send it to parent
    ProcessEvent(evt);         // sends to this and upwards
}

void DetectionPage::OnChangeCamera(wxCommandEvent& event)
{
    if (IsCameraSelected())
        PersistentToolTip::SetToolTip(m_devicesList, GetSelectedCamera());
    else
        PersistentToolTip::RemoveToolTip(m_devicesList);

    UpdateCameraFeedBtns();
}

void DetectionPage::OnImageSourceToFile(wxCommandEvent& event)
{
    LoadImagesUIMode();
}

void DetectionPage::OnImageSourceToCamera(wxCommandEvent& event)
{
    ImgAcqAssistantUIMode();
}

void DetectionPage::OnOpenCameraAssistant(wxCommandEvent& event)
{
    // Let the parent know this board was clicked
    wxCommandEvent evt(GUI_CAMERA_ASSISTANT, GetId());

    // Send it to parent
    ProcessEvent(evt);         // sends to this and upwards
}

void DetectionPage::OnLoadImage(wxCommandEvent& event)
{
    // Let the parent know this board was clicked
    wxCommandEvent evt(GUI_LOAD_IMAGE, GetId());

    // Send it to parent
    ProcessEvent(evt);         // sends to this and upwards
}

void DetectionPage::OnSnap(wxCommandEvent& event)
{
    // Let the parent know this board was clicked
    wxCommandEvent evt(GUI_SNAP_CAMERA, GetId());

    // Send it to parent
    ProcessEvent(evt);         // sends to this and upwards
}

void DetectionPage::OnLive(wxCommandEvent& event)
{
    // Let the parent know this board was clicked
    wxCommandEvent evt(GUI_LIVE_CAMERA, GetId());

    // Send it to parent
    ProcessEvent(evt);         // sends to this and upwards
}

void DetectionPage::OnAutoCapture(wxCommandEvent& event)
{
    // Let the parent know this board was clicked
    wxCommandEvent evt(GUI_AUTO_CAPTURE, GetId());
    evt.SetInt(m_autoCaptureCheckBox->GetValue());

    // Send it to parent
    ProcessEvent(evt);         // sends to this and upwards
}

void DetectionPage::OnSaveAllBoards(wxCommandEvent& event)
{
    // Let the parent know this board was clicked
    wxCommandEvent evt(GUI_SAVE_IMAGE, GetId());

    // Send it to parent
    ProcessEvent(evt);         // sends to this and upwards
}

void DetectionPage::OnImagePreProcess(wxCommandEvent& event)
{
    m_imagePreprocessFrame->Open();
}

void DetectionPage::OnAcceptDataset(wxCommandEvent& event)
{
    // Let the parent know this board was clicked
    wxCommandEvent evt(GUI_ACCEPT_DATASET, GetId());

    // Send it to parent
    ProcessEvent(evt);         // sends to this and upwards
}
