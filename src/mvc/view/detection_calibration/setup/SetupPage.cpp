#include "SetupPage.hpp"
#include <wx/sizer.h>
#include <wx/image.h>
#include <wx/bmpbuttn.h>
#include <wx/radiobut.h>
#include <wx/stattext.h>
#include <gui_elements/persistent_tooltip/PersistentToolTip.hpp>
#include <gui_elements/parameter/ParameterWidgetList.hpp>
#include <gui_elements/parameter/ParameterWidget.hpp>
#include <parameter/ParameterInfo.hpp>
#include "SetupPageEvents.hpp"


///////////////////////////////////////////////////////////////////////

static const wxString AUTOMATIC_MODE_BRIEF{
    "This is the 'just make it work' path. It assumes no prior knowledge of the hardware and that you just want to start clicking.\n"
    "In practice, the algorithm passes empty matrices to 'cv::calibrateCamera', letting OpenCV's internal method find the true starting point from scratch.\n"
    "During detection, the app generates a heuristic initial guess for the camera matrix, based on the given image dimensions and safe standard guesses, in order to still provide a pose estimation:\n\n"
    "\tfx = fy = max(imageWidth, imageHeight)\n"
    "\tcx = imageWidth/2\n"
    "\tcy = imageHeight/2\n\n"
    "\t[fx   0  cx]\n"
    "\t[0   fy  cy]\n"
    "\t[0    0   1]\n\n"
    "Note: Pose estimation accuracy will be the lowest among the other modes."
};

static const wxString FROM_HARDWARE_MODE_BRIEF{
    "This mode strikes a balance between ease of use and mathematical convergence. By providing hardware specs, the optimization starts much closer to a plausible 'truth'.\n"
    "Will be generated a heuristic initial guess for the camera matrix, based on the provided image dimensions [px], sensor focal length [mm] and pixel pitch [mm]:\n\n"
    "\tfx = focalLen / pixelPitchWidth\n"
    "\tfy = focalLen / pixelPitchHeight\n"
    "\tcx = imageWidth/2\n"
    "\tcy = imageHeight/2\n\n"
    "\t[fx   0  cx]\n"
    "\t[0   fy  cy]\n"
    "\t[0    0   1]\n\n"
    "In practice, the algorithm passes these computed matrices to 'cv::calibrateCamera', letting OpenCV start with these intial guesses.\n"
    "Note: To ensure the optimizer respects your provided values, the 'Use initial guess' flag is automatically enabled and locked in the Calibration settings."
};

static const wxString FREE_MODE_BRIEF{
    "In this mode, all fields in the camera matrix and distortion vector are unlocked. You can manually type values or load them from file.\n"
    "Changing the distortion type will enable/disable the relevant coefficient fields.\n"
    "Note: To ensure the optimizer respects your provided values, you have to enable the 'Use initial guess' flag in the Calibration settings."
};

///////////////////////////////////////////////////////////////////////


SetupPage::SetupPage(
    wxWindow* parent,
    wxWindowID winid,
    const wxPoint& pos,
    const wxSize& size,
    long style,
    const wxString& name)
    : BaseView(this), wxPanel(parent, winid, pos, size, style, name)
{
    CreateTab();

    AutomaticMode();
}

///////////////////////////////////////////////////////////////////////

void SetupPage::CreateTab()
{
    // Use a vertical BoxSizer to stack the sections
    wxBoxSizer* vbox = new wxBoxSizer(wxVERTICAL);

    // Load and Save Settings
    wxBoxSizer* settingsSizer = new wxBoxSizer(wxHORIZONTAL);

    wxBitmap openBitmap(wxImage("resources/open.png", wxBITMAP_TYPE_PNG).Rescale(18, 18));
    m_loadConfig = new wxBitmapButton(this, wxID_ANY, openBitmap);
    m_loadConfig->SetToolTip("Load setup settings");
    m_loadConfig->Bind(wxEVT_BUTTON, &SetupPage::OnLoadConfig, this);

    settingsSizer->Add(m_loadConfig, 0);


    wxBitmap saveBitmap(wxImage("resources/save.png", wxBITMAP_TYPE_PNG).Rescale(18, 18));
    m_saveConfig = new wxBitmapButton(this, wxID_ANY, saveBitmap);
    m_saveConfig->SetToolTip("Save setup settings to file");
    m_saveConfig->Bind(wxEVT_BUTTON, &SetupPage::OnSaveConfig, this);
    settingsSizer->Add(m_saveConfig, 0);

    vbox->Add(settingsSizer, 0);

    // Calibration mode
    wxStaticBoxSizer* calibModeBox = new wxStaticBoxSizer(wxVERTICAL, this, "Calibration mode");

    m_automaticRadio = new wxRadioButton(this, wxID_ANY, "Automatic", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    m_automaticRadio->Bind(wxEVT_RADIOBUTTON, &SetupPage::OnAutomaticMode, this);

    m_fromHardwareRadio = new wxRadioButton(this, wxID_ANY, "From Hardware");
    m_fromHardwareRadio->Bind(wxEVT_RADIOBUTTON, &SetupPage::OnFromHardwareMode, this);

    m_freeRadio = new wxRadioButton(this, wxID_ANY, "Free");
    m_freeRadio->Bind(wxEVT_RADIOBUTTON, &SetupPage::OnFreeMode, this);

    calibModeBox->Add(m_automaticRadio, 0, wxALL, 0);
    calibModeBox->Add(m_fromHardwareRadio, 0, wxALL, 0);
    calibModeBox->Add(m_freeRadio, 0, wxEXPAND | wxALL, 0);

    vbox->Add(calibModeBox, 0, wxEXPAND | wxALL, 0);

    // Setup parameters
    wxStaticBoxSizer* modeBriefingBox = new wxStaticBoxSizer(wxVERTICAL, this, "Mode birefing");

    wxScrolledWindow* scrolled = new wxScrolledWindow(this, wxID_ANY);
    scrolled->SetScrollRate(5, 5);

    m_modeBriefing = new wxStaticText(scrolled, wxID_ANY, wxEmptyString);

    wxBoxSizer* scrollSizer = new wxBoxSizer(wxVERTICAL);
    scrollSizer->Add(m_modeBriefing, 0, wxALL, 0);

    scrolled->SetSizer(scrollSizer);
    scrolled->FitInside();

    modeBriefingBox->Add(scrolled, 1, wxEXPAND | wxALL, 0);

    vbox->Add(modeBriefingBox, 1, wxEXPAND | wxALL, 0);


    // Load and Save Model params
    wxBoxSizer* modelParamsSizer = new wxBoxSizer(wxHORIZONTAL);

    m_loadModelParams = new wxBitmapButton(this, wxID_ANY, openBitmap);
    m_loadModelParams->SetToolTip("Load model parameters");
    m_loadModelParams->Bind(wxEVT_BUTTON, &SetupPage::OnLoadModelParams, this);

    modelParamsSizer->Add(m_loadModelParams, 0);


    m_saveModelParams = new wxBitmapButton(this, wxID_ANY, saveBitmap);
    m_saveModelParams->SetToolTip("Save model parameters to file");
    m_saveModelParams->Bind(wxEVT_BUTTON, &SetupPage::OnSaveModelParams, this);
    modelParamsSizer->Add(m_saveModelParams, 0);

    vbox->Add(modelParamsSizer, 0);


    // Automatic mode

    // Camera intrinsics
    m_camIntrinsicsBox = new wxStaticBoxSizer(wxVERTICAL, this, "Camera intrinsics");

    m_camIntrinsicsList = new ParameterWidgetList(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE);

    m_camIntrinsicsBox->Add(m_camIntrinsicsList, 1, wxEXPAND | wxALL, 0);

    vbox->Add(m_camIntrinsicsBox, 1, wxEXPAND | wxALL, 0);


    // From hardware mode

    // Distortion model parameters
    m_distModelBox = new wxStaticBoxSizer(wxVERTICAL, this, "Distortion model parameters");

    m_distModelParamsList = new ParameterWidgetList(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE);

    m_distModelBox->Add(m_distModelParamsList, 1, wxEXPAND | wxALL, 0);

    vbox->Add(m_distModelBox, 1, wxEXPAND | wxALL, 0);


    // Free mode

    // Camera matrix
    m_camMatrixBox = new wxStaticBoxSizer(wxVERTICAL, this, "Camera matrix");

    m_camMatrixList = new ParameterWidgetList(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE);

    m_camMatrixBox->Add(m_camMatrixList, 1, wxEXPAND | wxALL, 0);

    vbox->Add(m_camMatrixBox, 1, wxEXPAND | wxALL, 0);


    // Set the sizer to the this
    SetSizer(vbox);
}

///////////////////////////////////////////////////////////////////////

void SetupPage::OnLoadConfig(wxCommandEvent& event)
{
    // Let the parent know
    wxCommandEvent evt(GUI_SETUP_LOAD_CONFIG, GetId());

    // Send it to parent
    ProcessEvent(evt);         // sends to this and upwards
}

void SetupPage::OnSaveConfig(wxCommandEvent& event)
{
    // Let the parent know
    wxCommandEvent evt(GUI_SETUP_SAVE_CONFIG, GetId());

    // Send it to parent
    ProcessEvent(evt);         // sends to this and upwards
}

void SetupPage::OnLoadModelParams(wxCommandEvent& event)
{
    // Let the parent know
    wxCommandEvent evt(GUI_SETUP_LOAD_MODEL_PARAMS, GetId());

    // Send it to parent
    ProcessEvent(evt);         // sends to this and upwards
}

void SetupPage::OnSaveModelParams(wxCommandEvent& event)
{
    // Let the parent know
    wxCommandEvent evt(GUI_SETUP_SAVE_MODEL_PARAMS, GetId());

    // Send it to parent
    ProcessEvent(evt);         // sends to this and upwards
}

void SetupPage::OnAutomaticMode(wxCommandEvent& event)
{
    AutomaticMode();

    // Let the parent know this mode was clicked
    wxCommandEvent evt(GUI_AUTOMATIC_MODE, GetId());

    // Send it to parent
    ProcessEvent(evt);         // sends to this and upwards
}

void SetupPage::OnFromHardwareMode(wxCommandEvent& event)
{
    FromHardwareMode();

    // Let the parent know this mode was clicked
    wxCommandEvent evt(GUI_FROM_HARDWARE_MODE, GetId());

    // Send it to parent
    ProcessEvent(evt);         // sends to this and upwards
}

void SetupPage::OnFreeMode(wxCommandEvent& event)
{
    FreeMode();

    // Let the parent know this mode was clicked
    wxCommandEvent evt(GUI_FREE_MODE, GetId());

    // Send it to parent
    ProcessEvent(evt);         // sends to this and upwards
}

///////////////////////////////////////////////////////////////////////

void SetupPage::SetParameters(
    const std::vector<std::shared_ptr<ParameterInfo>>& params,
    ParameterLocation where)
{
    switch (where)
    {
    case ParameterLocation::DISTORTION_MODEL:
        m_distModelParamsList->SetParameters(params);

        break;

    case ParameterLocation::CAMERA_INTRINSICS:
        m_camIntrinsicsList->SetParameters(params);

        break;

    case ParameterLocation::CAMERA_MATRIX:
        m_camMatrixList->SetParameters(params);

        break;
    }
}

void SetupPage::UpdateParameter(const std::shared_ptr<ParameterInfo>& param)
{
	const wxString paramId = param->name();

	std::shared_ptr<ParameterWidget> widget = m_distModelParamsList->GetWidget(paramId);
	if (widget)
		widget->Update(param);
    
    widget = m_camIntrinsicsList->GetWidget(paramId);
	if (widget)
		widget->Update(param);
    
    widget = m_camMatrixList->GetWidget(paramId);
	if (widget)
		widget->Update(param);
}

void SetupPage::MarkParameterAsDirty(const wxString& paramId, bool isDirty)
{
    std::shared_ptr<ParameterWidget> widget = m_distModelParamsList->GetWidget(paramId);
    if (widget)
        widget->MarkAsDirty(isDirty);

    widget = m_camIntrinsicsList->GetWidget(paramId);
    if (widget)
        widget->MarkAsDirty(isDirty);

    widget = m_camMatrixList->GetWidget(paramId);
    if (widget)
        widget->MarkAsDirty(isDirty);
}

///////////////////////////////////////////////////////////////////////

void SetupPage::AutomaticMode()
{
    m_automaticRadio->SetValue(true);

    m_modeBriefing->SetLabel(AUTOMATIC_MODE_BRIEF);

    m_camMatrixBox->ShowItems(false);

    Layout();
}

void SetupPage::FromHardwareMode()
{
    m_fromHardwareRadio->SetValue(true);

    m_modeBriefing->SetLabel(FROM_HARDWARE_MODE_BRIEF);

    m_camMatrixBox->ShowItems(false);

    Layout();
}

void SetupPage::FreeMode()
{
    m_freeRadio->SetValue(true);

    m_modeBriefing->SetLabel(FREE_MODE_BRIEF);

    m_camMatrixBox->ShowItems(true);

    Layout();
}