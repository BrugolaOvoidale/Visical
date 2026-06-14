#include "EvaluationPanel.hpp"
#include <wx/sizer.h>
#include <wx/image.h>
#include <wx/bmpbuttn.h>
#include <wx/spinctrl.h>
#include <evaluator/PluginContextInfo.hpp>
#include <evaluator/PluginInfo.hpp>
#include <evaluator/PluginResult.hpp>
#include "EvaluationPluginWidget.hpp"
#include "EvaluationPluginWidgetList.hpp"
#include "EvaluationPluginWidgetEvents.hpp"
#include "EvaluationPanelConfigFrame.hpp"
#include "PluginPreviewWidgetEvents.hpp"
#include "EvaluationPanelEvents.hpp"


//////////////////////////////////////////////

static const wxString LOAD_ICON{ "resources/open.png" };
static const wxString SAVE_ICON{ "resources/save.png" };
static const wxString GEAR_ICON{ "resources/gear.png" };

/////////////////////////////////////////////


EvaluationPanel::EvaluationPanel(
	wxWindow* parent,
	wxWindowID winid,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxString& name)
	: wxPanel(parent, winid, pos, size, style, name)
{
    // Evaluation Section
    wxStaticBoxSizer* evaluationSection = new wxStaticBoxSizer(wxVERTICAL, this, "Evaluation");

    // Single Board Evaluation Section
    wxStaticBoxSizer* singleBoardSection = new wxStaticBoxSizer(wxHORIZONTAL, this, "Board");

    // Layouts
    wxBoxSizer* singleBoardMainSizer = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer* singleBoardButtonSizer = new wxBoxSizer(wxHORIZONTAL);

    // Load Board Plugins Button
    wxImage openImage(LOAD_ICON, wxBITMAP_TYPE_PNG);
    openImage.Rescale(18, 18);

    m_loadBoardPlugins = new wxBitmapButton(this, wxID_ANY, openImage);
    m_loadBoardPlugins->SetToolTip("Load single board quality plugins from file");
    m_loadBoardPlugins->Bind(wxEVT_BUTTON, &EvaluationPanel::OnBoardLoadPlugins, this);

    singleBoardButtonSizer->Add(m_loadBoardPlugins, 0, wxLEFT, 0);

    // Save Board Plugins Button
    wxImage saveImage(SAVE_ICON, wxBITMAP_TYPE_PNG);
    saveImage.Rescale(18, 18);

    m_saveBoardPlugins = new wxBitmapButton(this, wxID_ANY, saveImage);
    m_saveBoardPlugins->SetToolTip("Save single board quality plugins to file");
    m_saveBoardPlugins->Bind(wxEVT_BUTTON, &EvaluationPanel::OnBoardSavePlugins, this);

    singleBoardButtonSizer->Add(m_saveBoardPlugins, 0, wxLEFT, 0);

    // Config Board Plugins Button
    wxImage plusImage(GEAR_ICON, wxBITMAP_TYPE_PNG);
    plusImage.Rescale(18, 18);

    m_configBoardPlugins = new wxBitmapButton(this, wxID_ANY, plusImage);
    m_configBoardPlugins->SetToolTip("Manage plugins");
    m_configBoardPlugins->Bind(wxEVT_BUTTON, &EvaluationPanel::OnBoardOpenConfigFrame, this);

    singleBoardButtonSizer->Add(m_configBoardPlugins, 0, wxLEFT, 0);

    // Add stretcher to push controls to the right
    singleBoardButtonSizer->AddStretchSpacer(1);

    // Global Threshold 
    wxBoxSizer* boardThrSizer = new wxBoxSizer(wxHORIZONTAL);

    m_applyGlobalBoardThr = new wxButton(this, wxID_ANY, "Apply");
    m_applyGlobalBoardThr->Bind(wxEVT_BUTTON, &EvaluationPanel::OnBoardGlobalThr, this);
    m_globalBoardThr = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER);
    // TODO: Threshold should not be set by the View
    m_globalBoardThr->SetValue("70");

    boardThrSizer->Add(m_applyGlobalBoardThr, 0, wxALIGN_CENTER_VERTICAL | wxALL, 0);
    boardThrSizer->Add(m_globalBoardThr, 0, wxALIGN_CENTER_VERTICAL | wxALL, 0);

    singleBoardButtonSizer->Add(boardThrSizer, 0, wxEXPAND, 0);

    // Board Plugins List
    m_boardPluginsList = new EvaluationPluginWidgetList(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE);
    m_boardPluginsList->SetMinSize(FromDIP(wxSize(280, 40)));
    m_boardPluginsList->Bind(GUI_OPEN_PLUGIN_PARAMS, &EvaluationPanel::OnBoardOpenParamsFrame, this);
    m_boardPluginsList->Bind(GUI_SELECT_EVAL_PLUGIN, &EvaluationPanel::OnBoardSelectPlugin, this);
    m_boardPluginsList->Bind(GUI_SHOW_DEB_RESULT, &EvaluationPanel::OnBoardShowDebResult, this);

    // Combine Layouts
    singleBoardMainSizer->Add(singleBoardButtonSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 0);
    singleBoardMainSizer->Add(m_boardPluginsList, 1, wxEXPAND | wxALL, 0);

    singleBoardSection->Add(singleBoardMainSizer, 1, wxEXPAND | wxALL, 0);

    m_boardConfigFrame = new EvaluationPanelConfigFrame(this, "Plugins list");
    m_boardConfigFrame->Bind(GUI_ADD_PLUGIN, &EvaluationPanel::OnBoardAddPlugin, this);
    m_boardConfigFrame->Bind(GUI_REMOVE_PLUGIN, &EvaluationPanel::OnBoardRemovePlugin, this);


    // Board Sequence Evaluation Section
    wxStaticBoxSizer* boardSequenceSection = new wxStaticBoxSizer(wxHORIZONTAL, this, "Board Sequence");

    // Layouts
    wxBoxSizer* sequenceMainSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* sequenceButtonSizer = new wxBoxSizer(wxHORIZONTAL);

    // Load Board Sequence Plugins Button
    m_loadBoardSeqPlugins = new wxBitmapButton(this, wxID_ANY, openImage);
    m_loadBoardSeqPlugins->SetToolTip("Load board sequence quality plugins from file");
    m_loadBoardSeqPlugins->Bind(wxEVT_BUTTON, &EvaluationPanel::OnSequenceLoadPlugins, this);

    sequenceButtonSizer->Add(m_loadBoardSeqPlugins, 0, wxLEFT, 0);

    // Save Board Sequence Plugins Button
    m_saveBoardSeqPlugins = new wxBitmapButton(this, wxID_ANY, saveImage);
    m_saveBoardSeqPlugins->SetToolTip("Save board sequence quality plugins to file");
    m_saveBoardSeqPlugins->Bind(wxEVT_BUTTON, &EvaluationPanel::OnSequenceSavePlugins, this);

    sequenceButtonSizer->Add(m_saveBoardSeqPlugins, 0, wxLEFT, 0);

    // Config Board Sequence Plugins Button
    m_configBoardSeqPlugins = new wxBitmapButton(this, wxID_ANY, plusImage);
    m_configBoardSeqPlugins->SetToolTip("Manage plugins");
    m_configBoardSeqPlugins->Bind(wxEVT_BUTTON, &EvaluationPanel::OnSequenceOpenConfigFrame, this);

    sequenceButtonSizer->Add(m_configBoardSeqPlugins, 0, wxLEFT, 0);

    // Add stretcher to push controls to the far right
    sequenceButtonSizer->AddStretchSpacer(1);

    // Right-side controls container
    wxBoxSizer* rightControlsSizer = new wxBoxSizer(wxHORIZONTAL);

    // Threshold
    wxBoxSizer* boardSeqThrSizer = new wxBoxSizer(wxHORIZONTAL);

    m_applyGlobalBoardSeqThr = new wxButton(this, wxID_ANY, "Apply");
    m_applyGlobalBoardSeqThr->Bind(wxEVT_BUTTON, &EvaluationPanel::OnSequenceGlobalThr, this);
    m_globalBoardSeqThr = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER);
    // TODO: Threshold should not be set by the View
    m_globalBoardSeqThr->SetValue("70");

    boardSeqThrSizer->Add(m_applyGlobalBoardSeqThr, 0, wxALIGN_CENTER_VERTICAL | wxALL, 0);
    boardSeqThrSizer->Add(m_globalBoardSeqThr, 0, wxALIGN_CENTER_VERTICAL | wxALL, 0);

    rightControlsSizer->Add(boardSeqThrSizer, 0, wxALIGN_CENTER_VERTICAL | wxALL, 0);

    // Add the right controls container to the button sizer
    sequenceButtonSizer->Add(rightControlsSizer, 0, wxEXPAND, 0);

    // Board Sequence Plugins List
    m_boardSequencePluginsList = new EvaluationPluginWidgetList(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE);
    m_boardSequencePluginsList->SetMinSize(FromDIP(wxSize(280, 40)));
    m_boardSequencePluginsList->Bind(GUI_OPEN_PLUGIN_PARAMS, &EvaluationPanel::OnSequenceOpenParamsFrame, this);
    m_boardSequencePluginsList->Bind(GUI_SELECT_EVAL_PLUGIN, &EvaluationPanel::OnSequenceSelectPlugin, this);
    m_boardSequencePluginsList->Bind(GUI_SHOW_DEB_RESULT, &EvaluationPanel::OnSequenceShowDebResult, this);

    // Combine Layouts
    sequenceMainSizer->Add(sequenceButtonSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 0);
    sequenceMainSizer->Add(m_boardSequencePluginsList, 1, wxEXPAND | wxALL, 0);

    boardSequenceSection->Add(sequenceMainSizer, 1, wxEXPAND | wxALL, 0);

    m_sequenceConfigFrame = new EvaluationPanelConfigFrame(this, "Plugins list");
    m_sequenceConfigFrame->Bind(GUI_ADD_PLUGIN, &EvaluationPanel::OnSequenceAddPlugin, this);
    m_sequenceConfigFrame->Bind(GUI_REMOVE_PLUGIN, &EvaluationPanel::OnSequenceRemovePlugin, this);


    // Combine into Evaluation Section
    evaluationSection->Add(singleBoardSection, 1, wxEXPAND | wxALL, 0);
    evaluationSection->Add(boardSequenceSection, 1, wxEXPAND | wxALL, 0);


    SetSizer(evaluationSection);
}

//////////////////////////////////////////////

void EvaluationPanel::OnBoardAddPlugin(wxCommandEvent& event)
{
    wxCommandEvent evt(GUI_BOARD_ADD_PLUGIN, GetId());
    evt.SetString(event.GetString());

    ProcessEvent(evt);
}

void EvaluationPanel::OnBoardRemovePlugin(wxCommandEvent& event)
{
    wxCommandEvent evt(GUI_BOARD_REMOVE_PLUGIN, GetId());
    evt.SetString(event.GetString());

    ProcessEvent(evt);
}

void EvaluationPanel::OnBoardOpenParamsFrame(wxCommandEvent& event)
{
    m_boardSequencePluginsList->CloseLastParametersFrame();

    event.Skip();
}

void EvaluationPanel::OnBoardOpenConfigFrame(wxCommandEvent& event)
{
    m_boardConfigFrame->Open();
}

void EvaluationPanel::OnBoardSelectPlugin(wxCommandEvent& event)
{
    m_boardSequencePluginsList->UnselectWidget();

    event.Skip();
}

void EvaluationPanel::OnBoardShowDebResult(wxCommandEvent& event)
{
    m_boardSequencePluginsList->UncheckDebButton();

    wxCommandEvent evt(GUI_BOARD_SHOW_DEB_RESULT, GetId());
    evt.SetInt(event.GetInt());
    evt.SetString(event.GetString());
    ProcessEvent(evt);
    event.Skip();
}

void EvaluationPanel::OnBoardLoadPlugins(wxCommandEvent& event)
{
    // Let the parent know board plugins should be loaded
    wxCommandEvent evt(GUI_BOARD_LOAD_PLUGINS, GetId());

    // Send it to parent
    ProcessEvent(evt);

    event.Skip();
}

void EvaluationPanel::OnBoardSavePlugins(wxCommandEvent& event)
{
    // Let the parent know board plugins should be saved
    wxCommandEvent evt(GUI_BOARD_SAVE_PLUGINS, GetId());

    // Send it to parent
    ProcessEvent(evt);

    event.Skip();
}

void EvaluationPanel::OnBoardGlobalThr(wxCommandEvent& event)
{
    const int threshold = m_globalBoardThr->GetValue();

    // Let the parent know board global threshold was changed
    wxCommandEvent evt(GUI_BOARD_GLOBAL_THR, GetId());
    evt.SetInt(threshold);

    // Send it to parent
    ProcessEvent(evt);

    event.Skip();
}


void EvaluationPanel::OnSequenceAddPlugin(wxCommandEvent& event)
{
    wxCommandEvent evt(GUI_SEQUENCE_ADD_PLUGIN, GetId());
    evt.SetString(event.GetString());

    ProcessEvent(evt);
}

void EvaluationPanel::OnSequenceRemovePlugin(wxCommandEvent& event)
{
    wxCommandEvent evt(GUI_SEQUENCE_REMOVE_PLUGIN, GetId());
    evt.SetString(event.GetString());

    ProcessEvent(evt);
}

void EvaluationPanel::OnSequenceOpenParamsFrame(wxCommandEvent& event)
{
    m_boardPluginsList->CloseLastParametersFrame();

    event.Skip();
}

void EvaluationPanel::OnSequenceOpenConfigFrame(wxCommandEvent& event)
{
    m_sequenceConfigFrame->Open();
}

void EvaluationPanel::OnSequenceSelectPlugin(wxCommandEvent& event)
{
    m_boardPluginsList->UnselectWidget();

    event.Skip();
}

void EvaluationPanel::OnSequenceShowDebResult(wxCommandEvent& event)
{
    m_boardPluginsList->UncheckDebButton();

    wxCommandEvent evt(GUI_SEQUENCE_SHOW_DEB_RESULT, GetId());
    evt.SetInt(event.GetInt());
    evt.SetString(event.GetString());
    ProcessEvent(evt);
    event.Skip();
}

void EvaluationPanel::OnSequenceLoadPlugins(wxCommandEvent& event)
{
    // Let the parent know sequence plugins should be loaded
    wxCommandEvent evt(GUI_SEQUENCE_LOAD_PLUGINS, GetId());

    // Send it to parent
    ProcessEvent(evt);

    event.Skip();
}

void EvaluationPanel::OnSequenceSavePlugins(wxCommandEvent& event)
{
    // Let the parent know sequence plugins should be saved
    wxCommandEvent evt(GUI_SEQUENCE_SAVE_PLUGINS, GetId());

    // Send it to parent
    ProcessEvent(evt);

    event.Skip();
}

void EvaluationPanel::OnSequenceGlobalThr(wxCommandEvent& event)
{
    const int threshold = m_globalBoardSeqThr->GetValue();

    // Let the parent know sequence global threshold was changed
    wxCommandEvent evt(GUI_SEQUENCE_GLOBAL_THR, GetId());
    evt.SetInt(threshold);

    // Send it to parent
    ProcessEvent(evt);

    event.Skip();
}

//////////////////////////////////////////////

bool EvaluationPanel::IsShowDebPluginSelected() const
{
    return m_boardPluginsList->IsShowDebPluginSelected() || m_boardSequencePluginsList->IsShowDebPluginSelected();
}

bool EvaluationPanel::SetSelectedDebPlugin(
    const wxString& pluginId,
    PluginLocation where)
{
    bool success = false;
    switch (where)
    {
    case PluginLocation::PER_BOARD:
        success = m_boardPluginsList->CheckDebButton(pluginId);

        break;

    case PluginLocation::PER_SEQUENCE:
        success = m_boardSequencePluginsList->CheckDebButton(pluginId);

        break;
    }

    return success;
}

void EvaluationPanel::MarkPluginParameterAsDirty(
    const wxString& pluginId,
    const wxString& paramId,
    bool isDirty)
{
    std::shared_ptr<EvaluationPluginWidget> widget = m_boardPluginsList->GetWidget(pluginId);

    if (widget)
    {
        widget->MarkParameterAsDirty(paramId, isDirty);
    }


    widget = m_boardSequencePluginsList->GetWidget(pluginId);

    if (widget)
    {
        widget->MarkParameterAsDirty(paramId, isDirty);
    }
}

std::optional<std::pair<wxString, EvaluationPanel::PluginLocation>> EvaluationPanel::GetSelectedDebPlugin() const
{
    std::optional<wxString> pluginId = m_boardPluginsList->GetSelectedDebPlugin();
    if (pluginId.has_value())
        return std::make_pair(pluginId.value(), EvaluationPanel::PluginLocation::PER_BOARD);

    pluginId = m_boardSequencePluginsList->GetSelectedDebPlugin();
    if (pluginId.has_value())
        return std::make_pair(pluginId.value(), EvaluationPanel::PluginLocation::PER_SEQUENCE);

    return std::nullopt;
}

void EvaluationPanel::EnablePlugin(
    const wxString& pluginId,
    bool enable)
{
    std::shared_ptr<EvaluationPluginWidget> widget = m_boardPluginsList->GetWidget(pluginId);

    if (widget)
    {
        widget->SetActive(enable);
    }


    widget = m_boardSequencePluginsList->GetWidget(pluginId);

    if (widget)
    {
        widget->SetActive(enable);
    }
}

void EvaluationPanel::UpdatePlugin(const std::shared_ptr<PluginContextInfo>& pluginCtx)
{
	const wxString pluginId = pluginCtx->getPluginInfo()->id();

    std::shared_ptr<EvaluationPluginWidget> widget = m_boardPluginsList->GetWidget(pluginId);

    if (widget)
    {
        widget->UpdatePlugin(pluginCtx);
    }


    widget = m_boardSequencePluginsList->GetWidget(pluginId);

    if (widget)
    {
        widget->UpdatePlugin(pluginCtx);
    }
}

void EvaluationPanel::UpdatePluginResult(const std::shared_ptr<PluginResult>& pluginResult)
{
    const wxString pluginId = pluginResult->plugin()->id();

    std::shared_ptr<EvaluationPluginWidget> widget = m_boardPluginsList->GetWidget(pluginId);

    if (widget)
    {
        widget->UpdatePluginResult(pluginResult);
    }


    widget = m_boardSequencePluginsList->GetWidget(pluginId);

    if (widget)
    {
        widget->UpdatePluginResult(pluginResult);
    }
}

void EvaluationPanel::UpdateAllPluginsThreshold(
    double threshold,
    PluginLocation where)
{
    switch (where)
    {
        case PluginLocation::PER_BOARD:
        {
            std::vector<std::shared_ptr<EvaluationPluginWidget>> widgets = m_boardPluginsList->GetAllWidgets();
			for (auto& w : widgets) w->SetThreshold(threshold);

            break;
        }

        case PluginLocation::PER_SEQUENCE:
            std::vector<std::shared_ptr<EvaluationPluginWidget>> widgets = m_boardSequencePluginsList->GetAllWidgets();
            for (auto& w : widgets) w->SetThreshold(threshold);

            break;
        }
}

void EvaluationPanel::AppendPlugin(
    const std::shared_ptr<PluginContextInfo>& pluginCtx,
    PluginLocation where)
{
    switch (where)
    {
        case PluginLocation::PER_BOARD:
            m_boardPluginsList->AppendPlugin(pluginCtx);

            break;

        case PluginLocation::PER_SEQUENCE:
            m_boardSequencePluginsList->AppendPlugin(pluginCtx);

            break;
    }
}

void EvaluationPanel::EnableAllPluginsPreview(PluginLocation where)
{
    switch (where)
    {
    case PluginLocation::PER_BOARD:
        m_boardConfigFrame->EnableAllPluginsPreview();

        break;

    case PluginLocation::PER_SEQUENCE:
        m_sequenceConfigFrame->EnableAllPluginsPreview();

        break;
    }
}

void EvaluationPanel::EnablePluginPreview(const wxString& pluginId)
{
    m_boardConfigFrame->EnablePluginPreview(pluginId);
    m_sequenceConfigFrame->EnablePluginPreview(pluginId);
}

void EvaluationPanel::DisablePluginPreview(
    const wxString& pluginId,
    PluginLocation where)
{
    switch (where)
    {
    case PluginLocation::PER_BOARD:
        m_boardConfigFrame->DisablePluginPreview(pluginId);

        break;

    case PluginLocation::PER_SEQUENCE:
        m_sequenceConfigFrame->DisablePluginPreview(pluginId);

        break;
    }
}

void EvaluationPanel::SetPluginsPreview(
    const std::vector<std::shared_ptr<PluginContextInfo>>& plugins,
    EvaluationPanel::PluginLocation where)
{
    switch (where)
    {
    case PluginLocation::PER_BOARD:
        m_boardConfigFrame->SetPlugins(plugins);

        break;

    case PluginLocation::PER_SEQUENCE:
        m_sequenceConfigFrame->SetPlugins(plugins);

        break;
    }
}

void EvaluationPanel::ClearPlugin(const wxString& toClearId)
{
    std::shared_ptr<EvaluationPluginWidget> widget = m_boardPluginsList->GetWidget(toClearId);

    if (widget)
    {
        widget->ClearData();
    }


    widget = m_boardSequencePluginsList->GetWidget(toClearId);

    if (widget)
    {
        widget->ClearData();
    }
}

void EvaluationPanel::ClearAllPlugins(PluginLocation where)
{
    switch (where)
    {
        case PluginLocation::PER_BOARD:
        {
            std::vector<std::shared_ptr<EvaluationPluginWidget>> widgets = m_boardPluginsList->GetAllWidgets();
            for (auto& w : widgets) w->ClearData();

            break;
        }

        case PluginLocation::PER_SEQUENCE:
        {
            std::vector<std::shared_ptr<EvaluationPluginWidget>> widgets = m_boardSequencePluginsList->GetAllWidgets();
            for (auto& w : widgets) w->ClearData();

            break;
        }
    }
}

void EvaluationPanel::RemovePlugin(const wxString& toRemoveId)
{
    m_boardPluginsList->RemovePlugin(toRemoveId);
    m_boardSequencePluginsList->RemovePlugin(toRemoveId);
}

void EvaluationPanel::RemoveAllPlugins(PluginLocation where)
{
    switch (where)
    {
    case PluginLocation::PER_BOARD:
        m_boardPluginsList->RemoveAllWidgets();

        break;

    case PluginLocation::PER_SEQUENCE:
        m_boardSequencePluginsList->RemoveAllWidgets();

        break;
    }
}