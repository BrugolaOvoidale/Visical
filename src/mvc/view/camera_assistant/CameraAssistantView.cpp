#include "CameraAssistantView.hpp"
#include <wx/display.h>
#include <wx/gdicmn.h>
#include <wx/image.h>
#include <wx/menu.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/choice.h>
#include <wx/radiobut.h>
#include <wx/button.h>
#include <wx/textctrl.h>
#include <wx/bmpbuttn.h>
#include <wx/checkbox.h>
#include <gui_elements/busy_overlay/BusyOverlay.hpp>
#include <gui_elements/parameter/ParameterWidgetList.hpp>
#include <gui_elements/parameter/ParameterWidget.hpp>
#include <camera/ICameraInfo.hpp>
#include <parameter/ParameterInfo.hpp>
#include "camera/CameraWidgetList.hpp"
#include "CameraAssistantViewEvents.hpp"


CameraAssistantView::CameraAssistantView(
    const wxString& title,
    bool startShowed)
    : BaseView(this),
    wxFrame(nullptr, wxID_ANY, title)
{
    wxIcon icon;
    icon.LoadFile("resources/appIcon.ico", wxBITMAP_TYPE_ICO);
    SetIcon(icon);

    // Create the File menu
    wxMenu* fileMenu = new wxMenu;
    m_loadAppSettings = fileMenu->Append(wxID_OPEN, "&Open\tCtrl+O", "Open settings");
    m_saveAppSettings = fileMenu->Append(wxID_SAVE, "&Save\tCtrl+S", "Save current app settings");
    m_saveAppSettingsAs = fileMenu->Append(wxID_SAVEAS, "&Save As...\tCtrl+Shift+S", "Save current app settings as...");
    fileMenu->AppendSeparator();
    fileMenu->Append(wxID_EXIT, "E&xit\tAlt+F4", "Quit this program");

    // Create the Help menu
    wxMenu* helpMenu = new wxMenu;
    helpMenu->Append(wxID_ABOUT, "&About\tF1", "Show informations");

    // Create the menu bar and attach the menus
    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(fileMenu, "&File");
    menuBar->Append(helpMenu, "&Help");

    menuBar->SetBackgroundColour(GetBackgroundColour());

    // Attach the menu bar to the frame
    SetMenuBar(menuBar);

    wxPanel* mainPanel = new wxPanel(this);

    //
    wxPanel* connectionSourcePanel = CreateConnectionSourcePanel(mainPanel);

    //
    wxPanel* camerasPanel = CreateCamerasPanel(mainPanel);


    // Create a vertical sizer and add the button above the notebook.
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->Add(connectionSourcePanel, 0, wxEXPAND | wxALL, 0);
    mainSizer->Add(camerasPanel, 1, wxEXPAND | wxALL, 0);
    mainPanel->SetSizerAndFit(mainSizer);
    mainPanel->SetMinSize(mainPanel->GetSize());

    // DPI-aware minimum for the whole frame
    const wxSize kMinFrameSize = FromDIP(mainSizer->GetMinSize());
    SetMinClientSize(kMinFrameSize);

    wxDisplay display(wxDisplay::GetFromWindow(this));
    wxRect screenRect = display.GetClientArea();

    // Initial size: 80%x80% of screen, but never smaller than our minimum
    const int width = std::max((int)(screenRect.GetWidth() * 0.8), kMinFrameSize.x);
    const int height = std::max((int)(screenRect.GetHeight() * 0.6), kMinFrameSize.y);
    SetSize(wxSize(width, height));

    Centre();
    
    Bind(wxEVT_CLOSE_WINDOW, &CameraAssistantView::OnClose, this);
    //Bind(wxEVT_MENU, &CalibrationView::OnClose, this, wxID_EXIT);
    Bind(wxEVT_MENU, &CameraAssistantView::OnLoadSettings, this, wxID_OPEN);
    Bind(wxEVT_MENU, &CameraAssistantView::OnSaveSettings, this, wxID_SAVE);
    Bind(wxEVT_MENU, &CameraAssistantView::OnSaveSettingsAs, this, wxID_SAVEAS);

    Layout();

    Show(startShowed);
}

////////////////////////////////////////////////////////////////////////////////////////

void CameraAssistantView::Open()
{
    if (!IsIconized())  Restore();

    if (!IsShown())     Show();

    if (!HasFocus())    SetFocus();
}

void CameraAssistantView::SetCamerasConfigPath(const wxString& path)
{
    m_txtFilePath->SetLabel(path);
}

void CameraAssistantView::SetUiState(UiState uiState)
{
    switch (uiState)
    {
        case UiState::IDLE:
            IdleMode();

            break;

        case UiState::ENUMERATION:
            EnumerationMode();

            break;

        case UiState::LOADING_CAMERA_PARAMETERS:
            LoadingCameraParametersMode();

            break;

        default:
            IdleMode();

            break;
    }
}

void CameraAssistantView::SetCameraSource(Source source)
{
    switch (source)
    {
        case Source::ENUMERATION:
            DiscoveryFromEnumMode();

            break;

        case Source::FILE:
            DiscoveryFromFileMode();

            break;
    }
}

CameraAssistantView::Source CameraAssistantView::GetCameraSource() const
{
    return m_cameraSrc;
}

void CameraAssistantView::EnableAutoDiscovery(bool enable)
{
    m_autoDiscovery->SetValue(enable);
}

bool CameraAssistantView::GetAutoDiscovery()
{
    return m_autoDiscovery->GetValue();
}

void CameraAssistantView::SetDiscoveryBehaviours(const std::vector<wxString>& choices)
{
    m_discoveryBehaviour->Set(choices);
}

void CameraAssistantView::SelectDiscoveryBehaviour(const wxString& choice)
{
    m_discoveryBehaviour->SetStringSelection(choice);

}

void CameraAssistantView::UpdateCamera(const std::shared_ptr<ICameraInfo>& camera)
{
    std::shared_ptr widget = m_cameraList->GetWidget(camera->serialNumber());
    if (!widget)
        return;

    widget->Update(camera);
}

void CameraAssistantView::SetCameraOperation(
    const wxString& cameraId,
    CameraWidget::Operation camOp)
{
    std::shared_ptr widget = m_cameraList->GetWidget(cameraId);
    if (!widget)
        return;

    widget->SetCameraOperation(camOp);
}

void CameraAssistantView::AppendCamera(
    const std::shared_ptr<ICameraInfo>& camera,
    const wxArrayString& camBehaviours)
{
    m_cameraList->AppendCamera(camera, camBehaviours);
}

void CameraAssistantView::SelectCamera(const wxString& cameraId)
{
    m_cameraList->SelectCamera(cameraId);
}

std::optional<wxString> CameraAssistantView::GetSelectedCamera() const
{
    return m_cameraList->GetSelectedCamera();
}

void CameraAssistantView::RemoveCamera(const wxString& cameraId)
{
    m_cameraList->RemoveCamera(cameraId);
}

void CameraAssistantView::RemoveAllCameras()
{
    m_cameraList->RemoveAllWidgets();
}

void CameraAssistantView::SetCategories(const std::vector<wxString>& categories)
{
    m_categories->Clear();

    m_categories->Append(categories);

    m_paramsContentPanel->Layout();
}

void CameraAssistantView::SelectCategory(const wxString& category)
{
    m_categories->SetStringSelection(category);
}

wxString CameraAssistantView::GetSelectedCategory() const
{
    return m_categories->GetStringSelection();
}

void CameraAssistantView::SetVisibilities(const std::vector<wxString>& visibilities)
{
    m_visibilities->Clear();

    m_visibilities->Append(visibilities);

    m_paramsContentPanel->Layout();
}

void CameraAssistantView::SelectVisibility(const wxString& visibility)
{
    m_visibilities->SetStringSelection(visibility);
}

wxString CameraAssistantView::GetSelectedVisibility() const
{
    return m_visibilities->GetStringSelection();
}

void CameraAssistantView::SetParameters(const std::vector<std::shared_ptr<ParameterInfo>>& params)
{
    m_cameraParameterList->SetParameters(params);
}

void CameraAssistantView::UpdateParameter(const std::shared_ptr<ParameterInfo>& param)
{
    std::shared_ptr<ParameterWidget> widget = m_cameraParameterList->GetWidget(param->name());
    if (!widget)
        return;

    widget->Update(param);
}

void CameraAssistantView::MarkParameterAsDirty(const wxString& paramId, bool isDirty)
{
    std::shared_ptr<ParameterWidget> widget = m_cameraParameterList->GetWidget(paramId);
    if (!widget)
        return;

    widget->MarkAsDirty(isDirty);
}

void CameraAssistantView::RemoveAllParameters()
{
    m_cameraParameterList->RemoveAllWidgets();
}

void CameraAssistantView::SetInterfaces(const std::vector<wxString>& interfaces)
{
    m_choiceInterfaces->Clear();
    m_choiceInterfaces->Append(interfaces);
}

void CameraAssistantView::SetParamsPersistence(bool arePersistent)
{
    m_persistentEdits->SetValue(arePersistent);
}

////////////////////////////////////////////////////////////

wxPanel* CameraAssistantView::CreateConnectionSourcePanel(wxWindow* parent)
{
    // Main Content Area
    wxPanel* panel = new wxPanel(parent);
    panel->SetBackgroundColour(*wxWHITE);

    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Discovery settings (FIRST, empty for now)
    wxStaticBoxSizer* discoverySettingsSizer =
        new wxStaticBoxSizer(wxVERTICAL, panel, "Discovery settings");


    wxBoxSizer* openSizer = new wxBoxSizer(wxHORIZONTAL);

    m_autoDiscovery = new wxCheckBox(panel, wxID_ANY, "Auto discovery on startup");

    openSizer->Add(m_autoDiscovery, 0, wxLEFT, 0);


    wxBoxSizer* discoverySizer = new wxBoxSizer(wxHORIZONTAL);

    wxStaticText* discoveryLabel = new wxStaticText(panel, wxID_ANY, "Discovery behaviours");

    m_discoveryBehaviour = new wxChoice(panel, wxID_ANY);
    m_discoveryBehaviour->Bind(
        wxEVT_CHOICE,
        &CameraAssistantView::OnChangeDiscoveryBehaviour,
        this
    );

    discoverySizer->Add(discoveryLabel, 0, wxLEFT, 0);
    discoverySizer->Add(m_discoveryBehaviour, 0, wxLEFT, 0);

    discoverySettingsSizer->Add(openSizer, 0, wxLEFT, 0);
    discoverySettingsSizer->Add(discoverySizer, 0, wxLEFT, 0);

    mainSizer->Add(discoverySettingsSizer, 0, wxEXPAND | wxALL, 0);

    // Discovery actions (EVERYTHING ELSE GOES HERE)
    wxStaticBoxSizer* discoveryActionsSizer =
        new wxStaticBoxSizer(wxVERTICAL, panel, "Discovery actions");

    // Connection Source: From Discovery
    wxBoxSizer* fromDiscoverySizer = new wxBoxSizer(wxVERTICAL);

    m_rbFromEnumeration = new wxRadioButton(
        panel, wxID_ANY, "From enumeration", wxDefaultPosition, wxDefaultSize, wxRB_GROUP
    );
    m_rbFromEnumeration->Bind(wxEVT_RADIOBUTTON, &CameraAssistantView::OnDiscoverySourceFromEnumeration, this);
    fromDiscoverySizer->Add(m_rbFromEnumeration, 0, wxALL, 0);

    // Row: auto-discovery | interface list | discover button
    wxBoxSizer* discoveryRow = new wxBoxSizer(wxHORIZONTAL);

    m_btnAutoDiscovery = new wxButton(panel, wxID_ANY, "Auto-discovery");
    m_btnAutoDiscovery->Bind(wxEVT_BUTTON, &CameraAssistantView::OnAutoDiscovery, this);
    discoveryRow->Add(m_btnAutoDiscovery, 0, wxRIGHT, 0);

    m_choiceInterfaces = new wxChoice(panel, wxID_ANY);
    discoveryRow->Add(m_choiceInterfaces, 1, wxRIGHT, 0);

    m_btnDiscover = new wxButton(panel, wxID_ANY, "Discover");
    m_btnDiscover->Bind(wxEVT_BUTTON, &CameraAssistantView::OnDiscoverInterface, this);
    discoveryRow->Add(m_btnDiscover, 0);

    fromDiscoverySizer->Add(discoveryRow, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 0);
    discoveryActionsSizer->Add(fromDiscoverySizer, 0, wxEXPAND | wxALL, 0);

    // Connection Source: From File
    wxBoxSizer* fromFileSizer = new wxBoxSizer(wxVERTICAL);

    m_rbFromFile = new wxRadioButton(panel, wxID_ANY, "From file");
    m_rbFromFile->Bind(wxEVT_RADIOBUTTON, &CameraAssistantView::OnDiscoverySourceFromFile, this);
    fromFileSizer->Add(m_rbFromFile, 0, wxALL, 0);

    // Row: label | file path | browse button
    wxBoxSizer* fileRow = new wxBoxSizer(wxHORIZONTAL);

    m_lblFile = new wxStaticText(panel, wxID_ANY, "Cameras config:");
    fileRow->Add(m_lblFile, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 0);

    m_txtFilePath = new wxTextCtrl(
        panel, wxID_ANY, "", wxDefaultPosition, wxSize(220, -1), wxTE_READONLY
    );
    fileRow->Add(m_txtFilePath, 1, wxRIGHT, 0);

    m_btnBrowse = new wxButton(panel, wxID_ANY, "Browse...");
    m_btnBrowse->Bind(
        wxEVT_BUTTON,
        &CameraAssistantView::OnLoadCamerasFromFile,
        this
    );
    fileRow->Add(m_btnBrowse, 0);

    fromFileSizer->Add(fileRow, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 0);
    discoveryActionsSizer->Add(fromFileSizer, 0, wxEXPAND | wxALL, 0);

    // Finalize
    mainSizer->Add(discoveryActionsSizer, 0, wxEXPAND | wxALL, 0);

    panel->SetSizer(mainSizer);

    return panel;
}

wxPanel* CameraAssistantView::CreateCamerasPanel(wxWindow* parent)
{
    wxPanel* camerasPanel = new wxPanel(parent);
    camerasPanel->SetBackgroundColour(*wxWHITE);
    wxBoxSizer* hBox = new wxBoxSizer(wxHORIZONTAL);

    // Create separate panel for Cameras section
    wxPanel* camerasBoxPanel = new wxPanel(camerasPanel);
    wxStaticBoxSizer* camerasBox = new wxStaticBoxSizer(wxVERTICAL, camerasBoxPanel, "Cameras");

    // Create a content panel inside the static box
    wxPanel* camerasContentPanel = new wxPanel(camerasBoxPanel);

    // Save Camera list Button
    wxImage saveImage("resources/save.png", wxBITMAP_TYPE_PNG);
    saveImage.Rescale(18, 18);
    wxBitmap saveBitmap(saveImage);

    wxBitmapButton* saveCameraList = new wxBitmapButton(camerasContentPanel, wxID_ANY, saveBitmap);
    saveCameraList->SetToolTip("Save camera list to file");
    saveCameraList->Bind(wxEVT_BUTTON, &CameraAssistantView::OnSaveCamerasToFile, this);

    m_cameraList = new CameraWidgetList(camerasContentPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE);
    m_cameraList->SetMinSize(wxSize(500, 100));

    wxBoxSizer* camerasContentSizer = new wxBoxSizer(wxVERTICAL);
    camerasContentSizer->Add(saveCameraList, 0, wxLEFT, 0);
    camerasContentSizer->Add(m_cameraList, 1, wxEXPAND, 0);
    camerasContentPanel->SetSizer(camerasContentSizer);

    camerasBox->Add(camerasContentPanel, 1, wxEXPAND | wxALL, 0);
    camerasBoxPanel->SetSizer(camerasBox);
    hBox->Add(camerasBoxPanel, 1, wxEXPAND | wxALL, 0);

    // Create separate panel for Camera Parameters section
    wxPanel* cameraParametersPanel = new wxPanel(camerasPanel);
    wxStaticBoxSizer* cameraParametersBox = new wxStaticBoxSizer(wxVERTICAL, cameraParametersPanel, "Camera Parameters");

    // Create a content panel inside the static box
    m_paramsContentPanel = new wxPanel(cameraParametersPanel);
    wxBoxSizer* paramsContentSizer = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer* choicesSizer = new wxBoxSizer(wxHORIZONTAL);

    m_categories = new wxChoice(m_paramsContentPanel, wxID_ANY);
    m_categories->Bind(wxEVT_CHOICE, &CameraAssistantView::OnChangeCategory, this);

    m_visibilities = new wxChoice(m_paramsContentPanel, wxID_ANY);
    m_visibilities->Bind(wxEVT_CHOICE, &CameraAssistantView::OnChangeVisibility, this);

    m_persistentEdits = new wxCheckBox(m_paramsContentPanel, wxID_ANY, "Persistent edits");
    m_persistentEdits->SetToolTip("Check this box to make camera parameter edits persist when disconnected.\nNote that persistence may not survive power cycle on all parameters.");
    m_persistentEdits->Bind(wxEVT_CHECKBOX, &CameraAssistantView::OnChangeParamsPersitence, this);

    choicesSizer->Add(m_categories, 0, wxEXPAND | wxALL, 0);
    choicesSizer->Add(m_visibilities, 0, wxEXPAND | wxALL, 0);
    choicesSizer->Add(m_persistentEdits, 0, wxEXPAND | wxALL, 0);

    paramsContentSizer->Add(choicesSizer, 0, wxEXPAND | wxALL, 0);

    m_cameraParameterList = new ParameterWidgetList(m_paramsContentPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE);
    paramsContentSizer->Add(m_cameraParameterList, 1, wxEXPAND | wxALL, 0);
    m_paramsContentPanel->SetSizer(paramsContentSizer);

    cameraParametersBox->Add(m_paramsContentPanel, 1, wxEXPAND | wxALL, 0);
    cameraParametersPanel->SetSizer(cameraParametersBox);
    m_paramsContentPanel->SetMinSize(wxSize(300, 100));
    hBox->Add(cameraParametersPanel, 1, wxEXPAND | wxALL, 0);

    camerasPanel->SetSizer(hBox);

    // Create overlays on the content panels only (not the static box panels)
    m_busyOverlayCam = new BusyOverlay(camerasContentPanel);
    m_busyOverlayCamParams = new BusyOverlay(m_paramsContentPanel);

    return camerasPanel;
}

void CameraAssistantView::DiscoveryFromFileMode()
{
    if (m_cameraSrc == Source::FILE)
        return;

    m_cameraSrc = Source::FILE;

    m_rbFromFile->SetValue(true);
    m_rbFromEnumeration->SetValue(false);

    // Disable discovery controls
    m_btnAutoDiscovery->Disable();
    m_choiceInterfaces->Disable();
    m_btnDiscover->Disable();

    // Enable file controls
    m_lblFile->Enable();
    m_txtFilePath->Enable();
    m_btnBrowse->Enable();
}

void CameraAssistantView::DiscoveryFromEnumMode()
{
    if (m_cameraSrc == Source::ENUMERATION)
        return;

    m_cameraSrc = Source::ENUMERATION;

    m_rbFromFile->SetValue(false);
    m_rbFromEnumeration->SetValue(true);

    // Enable discovery controls
    m_btnAutoDiscovery->Enable();
    m_choiceInterfaces->Enable();
    m_btnDiscover->Enable();

    // Disable file controls
    m_lblFile->Disable();
    m_txtFilePath->Disable();
    m_btnBrowse->Disable();
}

void CameraAssistantView::IdleMode()
{
    m_busyOverlayCam->ShowOverlay(false);

    m_busyOverlayCamParams->ShowOverlay(false);
}

void CameraAssistantView::EnumerationMode()
{
    m_busyOverlayCam->UseSpinner();
    m_busyOverlayCamParams->UseSpinner();

    m_busyOverlayCam->ShowOverlay(true);
    m_busyOverlayCamParams->ShowOverlay(true);
}

void CameraAssistantView::LoadingCameraParametersMode()
{
    m_busyOverlayCam->UseStaticText(wxEmptyString);
    m_busyOverlayCamParams->UseStaticText("Loading parameters...");

    m_busyOverlayCam->ShowOverlay(true);
    m_busyOverlayCamParams->ShowOverlay(true);
}

void CameraAssistantView::OnLoadSettings(wxCommandEvent& event)
{
    // Let the parent know this button was clicked
    wxCommandEvent evt(GUI_CAM_ASSIST_LOAD_SETTINGS, GetId());

    // Send it to parent
    ProcessEvent(evt);         // sends to this and upwards
}

void CameraAssistantView::OnSaveSettings(wxCommandEvent& event)
{
    // Let the parent know this button was clicked
    wxCommandEvent evt(GUI_CAM_ASSIST_SAVE_SETTINGS, GetId());

    // Send it to parent
    ProcessEvent(evt);         // sends to this and upwards
}

void CameraAssistantView::OnSaveSettingsAs(wxCommandEvent& event)
{
    // Let the parent know this button was clicked
    wxCommandEvent evt(GUI_CAM_ASSIST_SAVE_SETTINGS_AS, GetId());

    // Send it to parent
    ProcessEvent(evt);         // sends to this and upwards
}

void CameraAssistantView::OnChangeDiscoveryBehaviour(wxCommandEvent& event)
{
    // Let the parent know this button was clicked
    wxCommandEvent evt(GUI_CHANGE_DISCOVERY_BEHAVIOUR, GetId());
    evt.SetString(m_discoveryBehaviour->GetStringSelection());

    // Send it to parent
    ProcessEvent(evt);         // sends to this and upwards
}

void CameraAssistantView::OnDiscoverySourceFromFile(wxCommandEvent& event)
{
    DiscoveryFromFileMode();
}

void CameraAssistantView::OnDiscoverySourceFromEnumeration(wxCommandEvent& event)
{
    DiscoveryFromEnumMode();
}

void CameraAssistantView::OnAutoDiscovery(wxCommandEvent& event)
{
    // Let the parent know this button was clicked
    wxCommandEvent evt(GUI_AUTO_DISCOVERY, GetId());

    // Send it to parent
    ProcessEvent(evt);         // sends to this and upwards
}

void CameraAssistantView::OnDiscoverInterface(wxCommandEvent& event)
{
    // Let the parent know this button was clicked
    wxCommandEvent evt(GUI_DISCOVER_INTERFACE, GetId());
    evt.SetString(m_choiceInterfaces->GetStringSelection());

    // Send it to parent
    ProcessEvent(evt);         // sends to this and upwards
}

void CameraAssistantView::OnLoadCamerasFromFile(wxCommandEvent& event)
{
    // Let the parent know this board was clicked
    wxCommandEvent evt(GUI_LOAD_CAMERAS_FILE, GetId());

    // Send it to parent
    ProcessEvent(evt);         // sends to this and upwards
}

void CameraAssistantView::OnSaveCamerasToFile(wxCommandEvent& event)
{
    // Let the parent know this board was clicked
    wxCommandEvent evt(GUI_SAVE_CAMERAS_FILE, GetId());

    // Send it to parent
    ProcessEvent(evt);         // sends to this and upwards
}

void CameraAssistantView::OnChangeCategory(wxCommandEvent& event)
{
    // Let the parent know that category changed
    wxCommandEvent evt(GUI_CHANGE_CATEGORY, GetId());
    evt.SetString(m_categories->GetStringSelection());

    // Send it to parent
    ProcessEvent(evt);         // sends to this and upwards
}

void CameraAssistantView::OnChangeVisibility(wxCommandEvent& event)
{
    // Let the parent know that visibility changed
    wxCommandEvent evt(GUI_CHANGE_VISIBILITY, GetId());
    evt.SetString(m_visibilities->GetStringSelection());

    // Send it to parent
    ProcessEvent(evt);         // sends to this and upwards
}

void CameraAssistantView::OnChangeParamsPersitence(wxCommandEvent& event)
{
    // Let the parent know that visibility changed
    wxCommandEvent evt(GUI_CHANGE_PARAMS_PERSISTENCE, GetId());
    evt.SetInt(m_persistentEdits->GetValue());

    // Send it to parent
    ProcessEvent(evt);         // sends to this and upwards
}

void CameraAssistantView::OnClose(wxCloseEvent& event)
{
    if (event.CanVeto())
    {
        event.Veto();

        Hide();

        return;
    }

    event.Skip(); // allow normal destruction
}