#include "CommandParameterWidget.hpp"
#include <wx/button.h>
#include <wx/panel.h>
#include "ParameterWidgetEvents.hpp"


CommandParameterWidget::CommandParameterWidget(
    wxWindow* parent,
    const std::shared_ptr<ParameterInfo>& param)
    : ParameterWidget(
        parent,
        param,
        false)
{
}

//////////////////////////////////////////////////////////////////////////

wxWindow* CommandParameterWidget::CreateValueControl()
{
    m_value = new wxButton(m_parameterHeader, wxID_ANY, "Execute");

    m_value->Bind(wxEVT_BUTTON, &CommandParameterWidget::OnSetParameter, this);

    return m_value;
}

void CommandParameterWidget::InitializeParameter()
{
    // Nothing to initialize for a command parameter
}

void CommandParameterWidget::UpdateImpl(const std::shared_ptr<ParameterInfo>& param)
{
    // Nothing to update for a command parameter
}

void CommandParameterWidget::DoSetValue()
{
	// Nothing to set for a command parameter, as it doesn't have a value to display
}

void CommandParameterWidget::RWModeImpl()
{
    m_value->Enable();
}

void CommandParameterWidget::ReadOnlyModeImpl()
{
    m_value->Disable();
}

void CommandParameterWidget::OnSetParameter(wxCommandEvent& event)
{
    // Let the parent know this board was clicked
    ParameterChangedEvent evt(GUI_SET_PARAM_VALUE, m_widgetPanel->GetId());
    evt.SetParameterId(GetParameterId());
    evt.SetCategoryId(GetCategoryId());

    StoreLatestSetParamEvent(evt, -1);
}
