#include "EvaluationPanelConfigFrame.hpp"
#include <wx/image.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <gui_elements/parameter/ParameterWidgetList.hpp>
#include "PluginPreviewWidget.hpp"
#include "PluginPreviewWidgetList.hpp"


EvaluationPanelConfigFrame::EvaluationPanelConfigFrame(
    wxWindow* parent,
    const wxString& title)
    : wxFrame(parent, wxID_ANY, title)
{
    wxIcon icon;
    icon.LoadFile("resources/appIcon.ico", wxBITMAP_TYPE_ICO);
    SetIcon(icon);


    BuildUI();

    Hide();

    Bind(wxEVT_CLOSE_WINDOW, &EvaluationPanelConfigFrame::OnClose, this);
}

////////////////////////////////////////////////////////////

void EvaluationPanelConfigFrame::Open()
{
    if (!IsIconized())  Restore();

    if (!IsShown())     Show();

    if (!HasFocus())    SetFocus();
}

void EvaluationPanelConfigFrame::SetPlugins(const std::vector<std::shared_ptr<PluginContextInfo>>& plugins)
{
    m_pluginPreviewWidgetList->SetPlugins(plugins);
}

void EvaluationPanelConfigFrame::EnableAllPluginsPreview()
{
	std::vector<std::shared_ptr<PluginPreviewWidget>> widgets = m_pluginPreviewWidgetList->GetAllWidgets();
    for (const auto& w : widgets) w->EnablePlugin();
}

void EvaluationPanelConfigFrame::EnablePluginPreview(const wxString& pluginId)
{
    std::shared_ptr<PluginPreviewWidget> widget = m_pluginPreviewWidgetList->GetWidget(pluginId);
    if (widget)
        widget->EnablePlugin();
}

void EvaluationPanelConfigFrame::DisablePluginPreview(const wxString& pluginId)
{
    std::shared_ptr<PluginPreviewWidget> widget = m_pluginPreviewWidgetList->GetWidget(pluginId);
    if (widget)
        widget->DisablePlugin();
}

////////////////////////////////////////////////////////////

void EvaluationPanelConfigFrame::BuildUI()
{
    // Use a vertical BoxSizer to stack the sections
    wxBoxSizer* vbox = new wxBoxSizer(wxVERTICAL);

    // Plugins 
    wxStaticBoxSizer* paramsBox = new wxStaticBoxSizer(wxVERTICAL, this, "Plugins");

    m_pluginPreviewWidgetList = new PluginPreviewWidgetList(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE);
    m_pluginPreviewWidgetList->SetMinSize(FromDIP(wxSize(500, 200)));

    paramsBox->Add(m_pluginPreviewWidgetList, 1, wxEXPAND | wxALL, 0);

    vbox->Add(paramsBox, 1, wxEXPAND | wxALL, 0);  // Add to vertical layout

    SetBackgroundColour(*wxWHITE);

    // Set the sizer to the panel
    SetSizerAndFit(vbox);

    SetMinSize(GetSize());

    SetSize(GetSize());
}

void EvaluationPanelConfigFrame::OnClose(wxCloseEvent& event)
{
    if (event.CanVeto())
    {
        event.Veto();

        Hide();

        return;
    }

    event.Skip(); // allow normal destruction
}