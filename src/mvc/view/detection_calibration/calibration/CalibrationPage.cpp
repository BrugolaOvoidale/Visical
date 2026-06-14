#include "CalibrationPage.hpp"
#include <wx/event.h>
#include <wx/notebook.h>
#include <wx/sizer.h>
#include <wx/bmpbuttn.h>
#include <wx/checkbox.h>
#include <wx/tglbtn.h>
#include <wx/stattext.h>
#include <gui_elements/image_panel/ImagePanel.hpp>
#include <gui_elements/parameter/ParameterWidgetList.hpp>
#include <gui_elements/parameter/ParameterWidget.hpp>
#include <gui_elements/busy_overlay/BusyOverlay.hpp>
#include <parameter/ParameterInfo.hpp>
#include "../common/board/BoardWidgetList.hpp"
#include "../common/board/BoardWidgetEvents.hpp"
#include "CalibrationPageEvents.hpp"


static const wxString CALIB_ERROR_NULL{ "--.--" };
static const wxString SELECTED_PAGE{ wxT("• ") };

///////////////////////////////////////////////////////////////////////


CalibrationPage::CalibrationPage(
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
    m_notebook->Bind(wxEVT_NOTEBOOK_PAGE_CHANGED, &CalibrationPage::OnPageChanged, this);

    mainSizer->Add(m_notebook, 1, wxEXPAND | wxALL, 0);

    SetSizer(mainSizer);

    SetUiState(UiState::IDLE);
}

///////////////////////////////////////////////////////////////////////

int CalibrationPage::GoToPage(Page page)
{
    wxWindow p;
    switch(page)
    {
        case Page::SETUP:
            return m_notebook->SetSelection(m_notebook->FindPage(m_setupDetectionTab));

            break;

        case Page::CALIBRATION:
            return m_notebook->SetSelection(m_notebook->FindPage(m_calibrationTab));

            break;
    }

    return wxNOT_FOUND;
}

void CalibrationPage::SetCalibrationError(double error)
{
    if (error < 0.0)
        m_calibError->SetLabel(CALIB_ERROR_NULL + " px");
    else
        m_calibError->SetLabel(wxString::Format("%.4f", error) + " px");
}

void CalibrationPage::SetParameters(
    const std::vector<std::shared_ptr<ParameterInfo>>& params,
    ParameterLocation where)
{
    switch (where)
    {
    case ParameterLocation::SETUP:
        m_calibParamsList->SetParameters(params);

        break;

    case ParameterLocation::CAMERA_MATRIX:
        m_camMatrixParamsList->SetParameters(params);

        break;

    case ParameterLocation::DISTORTION_MODEL:
        m_distModelParamsList->SetParameters(params);

        break;
    }
}

void CalibrationPage::UpdateParameter(const std::shared_ptr<ParameterInfo>& param)
{
    std::shared_ptr<ParameterWidget> widget = m_distModelParamsList->GetWidget(param->name());

    if (widget)
    {
        widget->Update(param);
    }


    widget = m_calibParamsList->GetWidget(param->name());

    if (widget)
    {
        widget->Update(param);
    }


    widget = m_camMatrixParamsList->GetWidget(param->name());

    if (widget)
    {
        widget->Update(param);
    }


    widget = m_distModelParamsList->GetWidget(param->name());

    if (widget)
    {
        widget->Update(param);
    }
}

void CalibrationPage::MarkParameterAsDirty(
    const wxString& paramId,
    bool isDirty)
{
    std::shared_ptr<ParameterWidget> widget = m_distModelParamsList->GetWidget(paramId);

    if (widget)
    {
        widget->MarkAsDirty(isDirty);
    }


    widget = m_calibParamsList->GetWidget(paramId);

    if (widget)
    {
        widget->MarkAsDirty(isDirty);
    }


    widget = m_camMatrixParamsList->GetWidget(paramId);

    if (widget)
    {
        widget->MarkAsDirty(isDirty);
    }


    widget = m_distModelParamsList->GetWidget(paramId);

    if (widget)
    {
        widget->MarkAsDirty(isDirty);
    }
}

void CalibrationPage::SetUiState(UiState uiState)
{
    m_uiState = uiState;

    switch (uiState)
    {
        case UiState::IDLE:
            IdleMode();

            break;

        case UiState::CALIBRATION:
            CalibrationMode();

            break;
    }
}

void CalibrationPage::CheckUndistort(bool check)
{
    m_undistortBtn->SetValue(check);
}

bool CalibrationPage::IsUndistortChecked() const
{
    return m_undistortBtn->GetValue();
}

void CalibrationPage::CheckAutoCalibrateOnNewDataset(bool check)
{
    m_autoCalibOnNewDataset->SetValue(check);
}

bool CalibrationPage::IsAutoCalibrateOnNewDatasetChecked() const
{
    return m_autoCalibOnNewDataset->GetValue();
}

//////////////////////////////////////////////////////////////////////////////////////

wxNotebook* CalibrationPage::CreateNotebook()
{
    wxNotebook* notebook = new wxNotebook(this, wxID_ANY);

    m_setupDetectionTab = CreateSetupCalibrationTab(notebook);

    m_calibrationTab = CreateCalibrationSubTab(notebook);

    notebook->AddPage(m_setupDetectionTab, SELECTED_PAGE + "Setup", true);
    notebook->AddPage(m_calibrationTab, "Calibration");

    m_currPage = 0;

    return notebook;
}

wxPanel* CalibrationPage::CreateSetupCalibrationTab(wxWindow* parent)
{
    wxPanel* panel = new wxPanel(parent);

    // Use a vertical BoxSizer to stack the sections
    wxBoxSizer* vbox = new wxBoxSizer(wxVERTICAL);

    // Load and Save 
    wxBoxSizer* settingsSizer = new wxBoxSizer(wxHORIZONTAL);

    wxBitmap openBitmap(wxImage("resources/open.png", wxBITMAP_TYPE_PNG).Rescale(18, 18));
    m_loadConfig = new wxBitmapButton(panel, wxID_ANY, openBitmap);
    m_loadConfig->SetToolTip("Load Calibration settings");
    m_loadConfig->Bind(wxEVT_BUTTON, &CalibrationPage::OnLoadSettings, this);

    settingsSizer->Add(m_loadConfig, 0);


    wxBitmap saveBitmap(wxImage("resources/save.png", wxBITMAP_TYPE_PNG).Rescale(18, 18));
    m_saveConfig = new wxBitmapButton(panel, wxID_ANY, saveBitmap);
    m_saveConfig->SetToolTip("Save Calibration settings to file");
    m_saveConfig->Bind(wxEVT_BUTTON, &CalibrationPage::OnSaveSettings, this);
    settingsSizer->Add(m_saveConfig, 0);

    vbox->Add(settingsSizer, 0);


    // Image Source Box
    wxStaticBoxSizer* calibSettingsBox = new wxStaticBoxSizer(wxVERTICAL, panel, "Calibration settings");

    m_autoCalibOnNewDataset = new wxCheckBox(panel, wxID_ANY, "Auto-calibrate on new dataset");

    calibSettingsBox->Add(m_autoCalibOnNewDataset, 0, wxALL, 0);

    vbox->Add(calibSettingsBox, 0, wxEXPAND | wxALL, 0);


    // Load and Save Model params
    wxBoxSizer* modelParamsSizer = new wxBoxSizer(wxHORIZONTAL);

    m_loadModelParams = new wxBitmapButton(panel, wxID_ANY, openBitmap);
    m_loadModelParams->SetToolTip("Load model parameters");
    m_loadModelParams->Bind(wxEVT_BUTTON, &CalibrationPage::OnLoadModelConfig, this);

    modelParamsSizer->Add(m_loadModelParams, 0);


    m_saveModelParams = new wxBitmapButton(panel, wxID_ANY, saveBitmap);
    m_saveModelParams->SetToolTip("Save model parameters to file");
    m_saveModelParams->Bind(wxEVT_BUTTON, &CalibrationPage::OnSaveModelConfig, this);
    modelParamsSizer->Add(m_saveModelParams, 0);

    vbox->Add(modelParamsSizer, 0);


    // Calibration parameters
    wxStaticBoxSizer* camParamsBox = new wxStaticBoxSizer(wxVERTICAL, panel, "Calibration parameters");

    m_calibParamsList = new ParameterWidgetList(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE);

    camParamsBox->Add(m_calibParamsList, 1, wxEXPAND | wxALL, 0);

    vbox->Add(camParamsBox, 1, wxEXPAND | wxALL, 0);  // Add to vertical layout

    // Set the sizer to the panel
    panel->SetSizer(vbox);

    return panel;
}

wxPanel* CalibrationPage::CreateCalibrationSubTab(wxWindow* parent)
{
    wxPanel* panel = new wxPanel(parent);
    wxBoxSizer* mainContentSizer = new wxBoxSizer(wxHORIZONTAL);

    // Left side
    wxBoxSizer* leftSideSizer = new wxBoxSizer(wxVERTICAL);

    wxPanel* calibSummaryPanel = CreateCalibSummaryPanel(panel);
    leftSideSizer->Add(calibSummaryPanel, 1, wxEXPAND);

    wxPanel* imageDisplayPanel = CreateImageDisplayPanel(panel);
    leftSideSizer->Add(imageDisplayPanel, 0, wxEXPAND);

    mainContentSizer->Add(leftSideSizer, 1, wxEXPAND);

    // Right side
    wxBoxSizer* rightSideSizer = new wxBoxSizer(wxVERTICAL);

    wxPanel* datasetPanel = CreateDatasetPanel(panel);
    datasetPanel->SetMinSize(FromDIP(wxSize(280, 50)));
    rightSideSizer->Add(datasetPanel, 1, wxEXPAND);

    m_evaluationPanel = new EvaluationPanel(panel);
    m_evaluationPanel->SetMinSize(FromDIP(wxSize(280, 40)));
    rightSideSizer->Add(m_evaluationPanel, 1, wxEXPAND);

    mainContentSizer->Add(rightSideSizer, 1, wxEXPAND);

    panel->SetSizer(mainContentSizer);

    return panel;
}

wxPanel* CalibrationPage::CreateImageDisplayPanel(wxWindow* parent)
{
    wxPanel* panel = new wxPanel(parent);

    wxStaticBoxSizer* imagePanelBox =
        new wxStaticBoxSizer(wxVERTICAL, panel, "Image Display");

    m_imagePanel = new ImagePanel(panel);
    m_imagePanel->SetBackgroundColour(*wxWHITE);
    m_imagePanel->SetMinSize(FromDIP(wxSize(250, 250)));
    m_imagePanel->SetDisplayMode(ImagePanel::FIT_ASPECT_RATIO);

    imagePanelBox->Add(m_imagePanel, 1, wxEXPAND);

    wxBoxSizer* drawSizer = new wxBoxSizer(wxHORIZONTAL);

    m_drawBoard = new wxCheckBox(panel, wxID_ANY, "Draw board");
    m_drawBoard->Bind(wxEVT_CHECKBOX, &CalibrationPage::OnDrawBoard, this);

    m_drawMarks = new wxCheckBox(panel, wxID_ANY, "Draw marks");
    m_drawMarks->Bind(wxEVT_CHECKBOX, &CalibrationPage::OnDrawMarks, this);

    m_drawWCS = new wxCheckBox(panel, wxID_ANY, "Draw WCS");
    m_drawWCS->Bind(wxEVT_CHECKBOX, &CalibrationPage::OnDrawWCS, this);

    m_undistortBtn = new wxToggleButton(panel, wxID_ANY, "Undistort");
    m_undistortBtn->Bind(wxEVT_TOGGLEBUTTON, &CalibrationPage::OnUndistort, this);

    drawSizer->Add(m_drawBoard, 0, wxALIGN_CENTER_VERTICAL);
    drawSizer->Add(m_drawMarks, 0, wxALIGN_CENTER_VERTICAL);
    drawSizer->Add(m_drawWCS, 0, wxALIGN_CENTER_VERTICAL);
    drawSizer->Add(m_undistortBtn, 0, wxALIGN_CENTER_VERTICAL);

    imagePanelBox->Add(drawSizer, 0, wxEXPAND | wxTOP, 5);

    panel->SetSizer(imagePanelBox);

    return panel;
}

wxPanel* CalibrationPage::CreateCalibSummaryPanel(wxWindow* parent)
{
    wxPanel* panel = new wxPanel(parent);

    // Calibration Section
    wxStaticBoxSizer* calibSummaryBox = new wxStaticBoxSizer(wxVERTICAL, panel, "Calibration summary");

    wxImage saveImage("resources/save.png", wxBITMAP_TYPE_PNG);
    saveImage.Rescale(18, 18);
    wxBitmap saveBitmap(saveImage);

    m_saveCalibResultBtn = new wxBitmapButton(panel, wxID_ANY, saveBitmap);
    m_saveCalibResultBtn->SetToolTip("Save calibration result to file");
    m_saveCalibResultBtn->Bind(wxEVT_BUTTON, &CalibrationPage::OnSaveCalibrationResult, this);
    calibSummaryBox->Add(m_saveCalibResultBtn, 0);

    wxStaticText* calibErrorTxt = new wxStaticText(panel, wxID_ANY, "RMS error: ");
    m_calibError = new wxStaticText(panel, wxID_ANY, CALIB_ERROR_NULL + " px");

    wxBoxSizer* calibErrorSizer = new wxBoxSizer(wxHORIZONTAL);

    calibErrorSizer->Add(calibErrorTxt, 0);
    calibErrorSizer->Add(m_calibError, 0);

    calibSummaryBox->Add(calibErrorSizer, 0);


    // Calibration summaries List
    wxStaticBoxSizer* camMatrixBox = new wxStaticBoxSizer(wxHORIZONTAL, panel, "Camera matrix");

    m_camMatrixParamsList = new ParameterWidgetList(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE);
    m_camMatrixParamsList->SetMinSize(FromDIP(wxSize(280, 40)));
    m_busyOverlayCamMatrix = new BusyOverlay(m_camMatrixParamsList);

    camMatrixBox->Add(m_camMatrixParamsList, 1, wxEXPAND | wxALL, 0);


    wxStaticBoxSizer* distModelBox = new wxStaticBoxSizer(wxHORIZONTAL, panel, "Distortion model");

    m_distModelParamsList = new ParameterWidgetList(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE);
    m_distModelParamsList->SetMinSize(FromDIP(wxSize(280, 40)));
    m_busyOverlayDistModel = new BusyOverlay(m_distModelParamsList);

    distModelBox->Add(m_distModelParamsList, 1, wxEXPAND | wxALL, 0);


    calibSummaryBox->Add(camMatrixBox, 1, wxEXPAND | wxALL, 0);
    calibSummaryBox->Add(distModelBox, 1, wxEXPAND | wxALL, 0);

    panel->SetSizer(calibSummaryBox);

    return panel;
}

wxPanel* CalibrationPage::CreateDatasetPanel(wxWindow* parent)
{
    wxPanel* panel = new wxPanel(parent);

    // Calibration Section
    wxStaticBoxSizer* calibBox = new wxStaticBoxSizer(wxVERTICAL, panel, "Dataset");

    // Main Layout
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Top Row: Controls
    wxBoxSizer* topControlsSizer = new wxBoxSizer(wxHORIZONTAL);

    // Add stretcher to push calibrate button to the right
    topControlsSizer->AddStretchSpacer(1);

    // --- Calibrate Button ---
    m_calibrateBtn = new wxButton(panel, wxID_ANY, "Calibrate");
    m_calibrateBtn->Bind(wxEVT_BUTTON, &CalibrationPage::OnCalibrate, this);

    topControlsSizer->Add(m_calibrateBtn, 0, wxALIGN_CENTER_VERTICAL | wxALL, 0);

    // Bottom Row: Board List
    m_boardsList = new BoardWidgetList(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE);

    // Combine Layouts
    mainSizer->Add(topControlsSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 0);
    mainSizer->Add(m_boardsList, 1, wxEXPAND | wxALL, 0);

    calibBox->Add(mainSizer, 1, wxEXPAND | wxALL, 0);

    panel->SetSizer(calibBox);

    return panel;
}

void CalibrationPage::DatasetUpdated()
{
    UpdateCalibrateBtn();
}

void CalibrationPage::UpdateCalibrateBtn()
{
    const bool enable = m_datasetStatus.total > 0 && m_uiState == UiState::IDLE;

    if (m_calibrateBtn->IsEnabled() != enable)
        m_calibrateBtn->Enable(enable);
}

void CalibrationPage::IdleMode()
{
    m_evaluationPanel->Enable();

    UpdateCalibrateBtn();

    if (m_busyOverlayCamMatrix->IsShown()) m_busyOverlayCamMatrix->ShowOverlay(false);

    if (m_busyOverlayDistModel->IsShown()) m_busyOverlayDistModel->ShowOverlay(false);
}

void CalibrationPage::CalibrationMode()
{
    m_evaluationPanel->Disable();

    UpdateCalibrateBtn();

    m_busyOverlayCamMatrix->ShowOverlay();

    m_busyOverlayDistModel->ShowOverlay();
}

void CalibrationPage::OnCalibrate(wxCommandEvent& event)
{
    // Let the parent know
    wxCommandEvent evt(GUI_START_CALIBRATION, GetId());

    // Send it to parent
    ProcessEvent(evt);         // sends to this and upwards
}

void CalibrationPage::OnSaveCalibrationResult(wxCommandEvent& event)
{
    // Let the parent know
    wxCommandEvent evt(GUI_SAVE_CALIBRATION, GetId());

    // Send it to parent
    ProcessEvent(evt);         // sends to this and upwards
}

void CalibrationPage::OnUndistort(wxCommandEvent& event)
{
    // Let the parent know
    wxCommandEvent evt(GUI_UNDISTORT, GetId());
    evt.SetInt(m_undistortBtn->GetValue());

    // Send it to parent
    ProcessEvent(evt);         // sends to this and upwards
}

void CalibrationPage::OnClose(wxEvent& event)
{
    Hide();
}