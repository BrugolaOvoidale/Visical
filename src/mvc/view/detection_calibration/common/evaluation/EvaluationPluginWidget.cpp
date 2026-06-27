#include "EvaluationPluginWidget.hpp"
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/checkbox.h>
#include <wx/stattext.h>
#include <wx/spinctrl.h>
#include <wx/tglbtn.h>
#include <wx/statline.h>
#include <wx/textctrl.h>
#include <wx/valtext.h>
#include <wx/timer.h>
#include <evaluator/PluginContextInfo.hpp>
#include <evaluator/PluginInfo.hpp>
#include <evaluator/PluginResult.hpp>
#include <gui_elements/persistent_tooltip/PersistentToolTip.hpp>
#include <gui_elements/parameter/ParameterWidgetEvents.hpp>
#include "EvaluationPluginParametersFrame.hpp"
#include "PluginPreviewWidget.hpp"
#include "EvaluationPluginWidgetEvents.hpp"


static const wxString METADATA_SEVERITY{ "Check status" };
static const wxString METADATA_QUALITY_ISSUE_MSG{ "Quality message" };

///////////////////////////////////////////////

EvaluationPluginWidget::EvaluationPluginWidget(
    wxWindow* parent,
    const std::shared_ptr<PluginContextInfo>& pluginCtx)
    : BaseWidget(parent, true),
    m_pluginId(pluginCtx->getPluginInfo()->id()),
    m_debounceTimer(this)
{
    const std::shared_ptr<PluginInfo>& plugin = pluginCtx->getPluginInfo();

    const wxString wxPluginId = plugin->name();

    SetParameters(pluginCtx->getFilteredParams());

    SetDescription(plugin->description());

    SetActive(pluginCtx->isPluginEnabled());

    SetNote(wxPluginId);

    UnsetScore();

    SetThreshold(plugin->getThreshold());

    m_hasDebugResult = plugin->supportsDebugResults();

    Bind(wxEVT_TIMER, &EvaluationPluginWidget::OnDebounceTimer, this);
}

//////////////////////////////////////////////////////////////////////////

const wxString& EvaluationPluginWidget::GetPluginId() const
{
    return m_pluginId;
}

void EvaluationPluginWidget::UpdatePlugin(const std::shared_ptr<PluginContextInfo>& pluginCtx)
{
    if (!pluginCtx || pluginCtx->getPluginInfo()->id() != m_pluginId)
        return;

    SetParameters(pluginCtx->getFilteredParams());

    SetActive(pluginCtx->isPluginEnabled());

    const std::shared_ptr<PluginInfo>& plugin = pluginCtx->getPluginInfo();
    SetNote(plugin->name());

    SetThreshold(plugin->getThreshold());
}

void EvaluationPluginWidget::UpdatePluginResult(const std::shared_ptr<PluginResult>& pluginRes)
{
    if (!pluginRes)
        return;

    const std::shared_ptr<PluginInfo>& plugin = pluginRes->plugin();

    if (plugin->id() != m_pluginId) return;

    SetScore(pluginRes->score());

    std::unordered_map<wxString, wxString> metadata;
    wxColour colour;

    switch (pluginRes->severity())
    {
        case EvaluationSeverity::FAILED:
            metadata.emplace(METADATA_SEVERITY, "Failed");
            // lighter red
            colour = BaseWidget::HIGHLIGHT_RED;
            break;

        case EvaluationSeverity::CRITICAL:
            metadata.emplace(METADATA_SEVERITY, "Critical");
            // lighter orange
            colour = BaseWidget::HIGHLIGHT_ORANGE;
            break;

        case EvaluationSeverity::WARNING:
            metadata.emplace(METADATA_SEVERITY, "Warning");
            // pale yellow
            colour = BaseWidget::HIGHLIGHT_YELLOW;
            break;

        case EvaluationSeverity::OK:
            metadata.emplace(METADATA_SEVERITY, "Ok");
            // light green
            colour = BaseWidget::HIGHLIGHT_GREEN;
            break;

        case EvaluationSeverity::INSUFFICIENT_DATA:
            metadata.emplace(METADATA_SEVERITY, "Insufficient data");
            [[fallthrough]]; 
        default:
            // neutral light gray/white
            colour = BaseWidget::DEFAULT_GRAY;
            break;
    }

    metadata.emplace(METADATA_QUALITY_ISSUE_MSG, pluginRes->message());

    SetColour(colour);

    UpdateMetadata(metadata);
}

void EvaluationPluginWidget::MarkParameterAsDirty(
    const wxString& paramId,
    const wxString& categoryId,
    bool isDirty)
{
    m_pluginParametersFrame->MarkParameterAsDirty(paramId, categoryId, isDirty);
}

void EvaluationPluginWidget::SetActive(bool isActive)
{
    if (m_isActive == isActive)
        return;

    m_isActive = isActive;

    if (IsInitialized())
        DoSetActive();
}

bool EvaluationPluginWidget::IsActive() const
{
    return m_isActive;
}

void EvaluationPluginWidget::SetNote(const wxString& newNote)
{
    if (m_note == newNote)
        return;

    m_note = newNote;

    if (IsInitialized())
        DoSetNote();
}

const wxString& EvaluationPluginWidget::GetNote() const
{
    return m_note;
}

void EvaluationPluginWidget::UnsetScore()
{
    if (m_score == -1.0)
        return;

    m_score = -1.0; // invalid score

    if (IsInitialized())
        DoUnsetScore();
}

void EvaluationPluginWidget::SetScore(double score)
{
    if (m_score == score)
        return;

    if (score < 0.0)
    {
        UnsetScore();
        return;
    }

    m_score = score;

    if (IsInitialized())
        DoSetScore();
}

double EvaluationPluginWidget::GetScore() const
{
    return m_score;
}

void EvaluationPluginWidget::SetDebugResult(bool isChecked)
{
    if (m_isDebugResultOn == isChecked)
        return;

    m_isDebugResultOn = isChecked;

    if (IsInitialized())
        DoSetDebugResult();
}

bool EvaluationPluginWidget::IsDebugResultShown() const
{
    return m_isDebugResultOn;
}

void EvaluationPluginWidget::SetThreshold(double threshold)
{
    if (m_threshold == threshold)
        return;

	m_threshold = threshold;

    if (IsInitialized())
        DoSetThreshold();
}

double EvaluationPluginWidget::GetThreshold() const
{
    return m_threshold;
}

void EvaluationPluginWidget::SetParameters(const std::vector<std::shared_ptr<ParameterInfo>>& params)
{
    if (m_params == params)
		return;

	m_params = params;

    if (IsInitialized())
        DoSetParameters();
}

void EvaluationPluginWidget::EnableThreshold(bool enable)
{
    if (m_isThresholdEnabled == enable)
		return;

	m_isThresholdEnabled = enable;

    if (IsInitialized())
        DoEnableThreshold();
}

void EvaluationPluginWidget::ClearData()
{
    UpdateMetadata({});

    UnsetScore();

    SetColour(BaseWidget::DEFAULT_GRAY);
}

void EvaluationPluginWidget::OpenParametersFrame()
{
    m_pluginParametersFrame->Open();
}

void EvaluationPluginWidget::CloseParametersFrame()
{
    m_pluginParametersFrame->Close();
}

//////////////////////////////////////////////////////////////////////////

wxPanel* EvaluationPluginWidget::CreateHeader()
{
    // Header row
    wxPanel* header = new wxPanel(m_widgetPanel);

    m_headerSizer = new wxBoxSizer(wxHORIZONTAL);

    // Active checkbox
    m_isActiveItem = new wxCheckBox(header, wxID_ANY, wxEmptyString);
    m_isActiveItem->Bind(wxEVT_CHECKBOX, &EvaluationPluginWidget::OnEnable, this);
    BindSelectable(m_isActiveItem, true);
    m_headerSizer->Add(m_isActiveItem, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);


    // Separator
    wxStaticLine* sep1 = new wxStaticLine(
        header, wxID_ANY,
        wxDefaultPosition,
        m_widgetPanel->FromDIP(wxSize(1, 20)),
        wxLI_VERTICAL);
    m_headerSizer->Add(sep1, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);


    // Note text
    m_noteItem = new wxStaticText(
        header,
        wxID_ANY,
        wxEmptyString,
        wxDefaultPosition,
        wxDefaultSize,
        wxST_ELLIPSIZE_MIDDLE
    );
    m_noteItem->SetMinSize(m_widgetPanel->FromDIP(wxSize(150, -1)));
    BindSelectable(m_noteItem);
    m_headerSizer->Add(m_noteItem, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);


    // Score text
    m_scoreItem = new wxStaticText(
        header,
        wxID_ANY,
        wxT("--.-- %"),
        wxDefaultPosition,
        wxDefaultSize,
        wxALIGN_RIGHT
    );
    m_scoreItem->SetMinSize(m_widgetPanel->FromDIP(wxSize(60, -1)));
    BindSelectable(m_scoreItem);
    m_headerSizer->Add(m_scoreItem, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);


    // Stretch spacer pushes threshold to the right edge
    m_headerSizer->AddStretchSpacer();


    // Threshold spinner
    wxArrayString allowed;
    allowed.Add("0"); allowed.Add("1"); allowed.Add("2"); allowed.Add("3");
    allowed.Add("4"); allowed.Add("5"); allowed.Add("6"); allowed.Add("7");
    allowed.Add("8"); allowed.Add("9");
    allowed.Add(".");  // allow decimal point

    wxTextValidator doubleValidator(wxFILTER_INCLUDE_CHAR_LIST);
    doubleValidator.SetIncludes(allowed);

    m_thresholdItem = new wxSpinCtrlDouble(
        header,
        wxID_ANY,
        wxEmptyString,
        wxDefaultPosition,
        wxDefaultSize,
        wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER);

    m_thresholdItem->SetValidator(doubleValidator);

    m_thresholdItem->Bind(wxEVT_SPINCTRLDOUBLE, &EvaluationPluginWidget::OnChangeThreshold, this);
    m_thresholdItem->Bind(wxEVT_TEXT_ENTER, &EvaluationPluginWidget::OnChangeThreshold, this);
    m_thresholdItem->Bind(wxEVT_MOUSEWHEEL, &EvaluationPluginWidget::OnVetoMouseWheel, this);
    
    BindSelectable(m_thresholdItem, true);

    m_headerSizer->Add(m_thresholdItem, 0, wxALIGN_CENTER_VERTICAL);

    wxStaticText* pct = new wxStaticText(header, wxID_ANY, wxT("%"));

    m_headerSizer->Add(pct, 0, wxALIGN_CENTER_VERTICAL);

    m_headerSizer->AddStretchSpacer(1);

    m_showDebugResult = new wxToggleButton(header, wxID_ANY, "Show");
    m_showDebugResult->Bind(wxEVT_TOGGLEBUTTON, &EvaluationPluginWidget::OnShowDebugResult, this);
    m_showDebugResult->SetToolTip("Show debug results");
    BindSelectable(m_showDebugResult, true);

    m_headerSizer->Add(m_showDebugResult, 0, wxALIGN_CENTER_VERTICAL | wxRESERVE_SPACE_EVEN_IF_HIDDEN);


    wxBitmap gearBitmap = wxImage("resources/gear.png", wxBITMAP_TYPE_PNG).Rescale(18, 18);
    m_pluginParametersBtn = new wxBitmapButton(header, wxID_ANY, gearBitmap);
    m_pluginParametersBtn->SetMinSize(m_widgetPanel->FromDIP(m_pluginParametersBtn->GetBestSize()));
    m_pluginParametersBtn->SetToolTip("Edit plugin parameters");
    m_pluginParametersBtn->Bind(wxEVT_BUTTON, &EvaluationPluginWidget::OnOpenPluginParametersFrame, this);
    BindSelectable(m_pluginParametersBtn, true);

    m_headerSizer->Add(m_pluginParametersBtn, 0, wxALIGN_CENTER_VERTICAL);

    m_pluginParametersFrame = new EvaluationPluginParametersFrame(m_widgetPanel, GetNote());
    m_pluginParametersFrame->Bind(GUI_SET_PARAM_VALUE, &EvaluationPluginWidget::OnSetPluginParameter, this);
    m_pluginParametersFrame->Bind(GUI_RESET_PARAM, &EvaluationPluginWidget::OnResetPluginParameter, this);

    header->SetSizer(m_headerSizer);

    return header;
}

void EvaluationPluginWidget::InitializeImpl()
{
    DoSetActive();

    DoSetNote();

    if (m_score >= 0.0)
        DoSetScore();
    else
        DoUnsetScore();

    DoSetThreshold();

    DoSetParameters();

    DoEnableThreshold();

    m_showDebugResult->Show(m_hasDebugResult);
}

void EvaluationPluginWidget::SetDescriptionImpl()
{
    PersistentToolTip::SetToolTip(m_noteItem, m_description);

    PersistentToolTip::SetToolTip(m_scoreItem, m_description);
}

void EvaluationPluginWidget::DoSetActive()
{
    m_isActiveItem->SetValue(m_isActive);
}

void EvaluationPluginWidget::DoSetNote()
{
    m_noteItem->SetLabel(m_note);
}

void EvaluationPluginWidget::DoUnsetScore()
{
    m_scoreItem->SetLabel(wxT("--.-- %"));
}

void EvaluationPluginWidget::DoSetScore()
{
    m_scoreItem->SetLabel(wxString::Format("%.2f", m_score) + wxT(" %"));
}

void EvaluationPluginWidget::DoSetDebugResult()
{
    m_showDebugResult->SetValue(m_isDebugResultOn);
}

void EvaluationPluginWidget::DoSetThreshold()
{
    m_thresholdItem->SetValue(m_threshold);
}

void EvaluationPluginWidget::DoSetParameters()
{
    m_pluginParametersFrame->SetParameters(m_params);
}

void EvaluationPluginWidget::DoEnableThreshold()
{
    m_thresholdItem->Enable(m_isThresholdEnabled);
}

void EvaluationPluginWidget::OnSpecializedWidgetClick(wxMouseEvent& event)
{
    // Let the parent know this plugin was clicked
    wxCommandEvent evt(GUI_SELECT_EVAL_PLUGIN, m_widgetPanel->GetId());
    evt.SetClientData(wxUIntToPtr(GetWidgetId()));
    evt.SetString(GetPluginId());

    // Send it to parent
    ProcessEvent(evt);         // sends to this and upwards

    event.Skip();
}

void EvaluationPluginWidget::OnEnable(wxCommandEvent& event)
{
    const bool isChecked = m_isActiveItem->IsChecked();

    m_isActive = isChecked;

    // Let the parent know this plugin was toggled
    wxCommandEvent evt(GUI_ENABLE_DISABLE_EVAL_PLUGIN, m_widgetPanel->GetId());
    evt.SetString(GetPluginId());
    evt.SetInt(isChecked);

    // Send it to parent
    ProcessEvent(evt);         // sends to this and upwards

    event.Skip();
}

void EvaluationPluginWidget::OnOpenPluginParametersFrame(wxCommandEvent& event)
{
    OpenParametersFrame();

    // Let the parent know this plugin frame was opened
    wxCommandEvent evt(GUI_OPEN_PLUGIN_PARAMS, m_widgetPanel->GetId());
    evt.SetString(GetPluginId());

    // Send it to parent
    ProcessEvent(evt);         // sends to this and upwards

    event.Skip();
}

void EvaluationPluginWidget::OnShowDebugResult(wxCommandEvent& event)
{
    const bool isChecked = m_showDebugResult->GetValue();

    m_isDebugResultOn = isChecked;

    // Let the parent know this plugin was toggled
    wxCommandEvent evt(GUI_SHOW_DEB_RESULT, m_widgetPanel->GetId());
    evt.SetString(GetPluginId());
    evt.SetInt(isChecked);

    // Send it to parent
    ProcessEvent(evt);         // sends to this and upwards

    event.Skip();
}

void EvaluationPluginWidget::OnChangeThreshold(wxCommandEvent& event)
{
    m_debouncedThr = m_thresholdItem->GetValue();

    m_debounceTimer.StartOnce(800);
}

void EvaluationPluginWidget::OnSetPluginParameter(ParameterChangedEvent& event)
{
    ParameterChangedEvent clone(event);
    clone.SetEventType(GUI_SET_PLUGIN_PARAM_VALUE);
    clone.SetString(GetPluginId());

    // Send it to parent
    ProcessEvent(clone);         // sends to this and upwards
}

void EvaluationPluginWidget::OnResetPluginParameter(ParameterEvent& event)
{
    ParameterEvent clone(event);
    clone.SetEventType(GUI_RESET_PLUGIN_PARAM);
    clone.SetString(GetPluginId());

    // Send it to parent
    ProcessEvent(clone);         // sends to this and upwards
}

void EvaluationPluginWidget::OnDebounceTimer(wxTimerEvent&)
{
    if (m_debouncedThr.has_value() && m_debouncedThr.value() == GetThreshold())
        return;

    m_threshold = m_debouncedThr.value();
    m_debouncedThr.reset();


    // Let the parent know threshold changed
    wxCommandEvent evt(GUI_SET_THRESHOLD_EVAL_PLUGIN, m_widgetPanel->GetId());
    evt.SetString(GetPluginId());
    evt.SetInt(GetThreshold());

    // Send it to parent
    ProcessEvent(evt);         // sends to this and upwards
}