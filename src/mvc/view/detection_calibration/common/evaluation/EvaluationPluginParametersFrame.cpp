#include "EvaluationPluginParametersFrame.hpp"
#include <wx/panel.h>
#include <wx/image.h>
#include <wx/sizer.h>
#include <gui_elements/parameter/ParameterWidgetList.hpp>
#include <gui_elements/parameter/ParameterWidget.hpp>
#include <parameter/ParameterInfo.hpp>


EvaluationPluginParametersFrame::EvaluationPluginParametersFrame(
    wxWindow* parent,
    const wxString& title)
    : wxFrame(parent, wxID_ANY, title)
{
    wxIcon icon;
    icon.LoadFile("resources/appIcon.ico", wxBITMAP_TYPE_ICO);
    SetIcon(icon);

    BuildUI();

    Hide();

    Bind(wxEVT_CLOSE_WINDOW, &EvaluationPluginParametersFrame::OnClose, this);
}

////////////////////////////////////////////////////////////

void EvaluationPluginParametersFrame::Open()
{
    if (!IsIconized())  Restore();

    if (!IsShown())     Show();

    if (!HasFocus())    SetFocus();
}

void EvaluationPluginParametersFrame::SetParameters(const std::vector<std::shared_ptr<ParameterInfo>>& params)
{
    m_pluginParamsList->SetParameters(params);
}

void EvaluationPluginParametersFrame::UpdateParameter(const std::shared_ptr<ParameterInfo>& param)
{
    std::shared_ptr<ParameterWidget> widget = m_pluginParamsList->GetWidget(param->name());

    if (widget)
    {
        widget->Update(param);
    }
}

void EvaluationPluginParametersFrame::MarkParameterAsDirty(
    const wxString& paramId,
    bool isDirty)
{
    std::shared_ptr<ParameterWidget> widget = m_pluginParamsList->GetWidget(paramId);

    if (widget)
    {
        widget->MarkAsDirty(isDirty);
    }
}

///////////////////////////////////////////////////////////

void EvaluationPluginParametersFrame::BuildUI()
{
    // Use a vertical BoxSizer to stack the sections
    wxBoxSizer* vbox = new wxBoxSizer(wxVERTICAL);

    // Plugin parameters 
    wxStaticBoxSizer* paramsBox = new wxStaticBoxSizer(wxVERTICAL, this, "Parameters");

    m_pluginParamsList = new ParameterWidgetList(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE);
    m_pluginParamsList->SetMinSize(FromDIP(wxSize(500, 200)));

    paramsBox->Add(m_pluginParamsList, 1, wxEXPAND | wxALL, 0);

    vbox->Add(paramsBox, 1, wxEXPAND | wxALL, 0);  // Add to vertical layout

    SetBackgroundColour(*wxWHITE);

    // Set the sizer to the panel
    SetSizerAndFit(vbox);

    SetMinSize(GetSize());

    SetSize(GetSize());
}

void EvaluationPluginParametersFrame::OnClose(wxCloseEvent& event)
{
    if (event.CanVeto())
    {
        event.Veto();

        Hide();

        return;
    }

    event.Skip(); // allow normal destruction
}