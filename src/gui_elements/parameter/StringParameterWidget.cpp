#include "StringParameterWidget.hpp"
#include <wx/panel.h>
#include <wx/textctrl.h>
#include <parameter/string/ParameterStringInfo.hpp>
#include "ParameterWidgetEvents.hpp"


StringParameterWidget::StringParameterWidget(
    wxWindow* parent,
    const std::shared_ptr<ParameterStringInfo>& param)
    : ParameterWidget(
        parent,
        param)
{
    SetValue(param->getValue());
}

//////////////////////////////////////////////////////////////////////////

void StringParameterWidget::SetValue(const wxString& newValue)
{
    if (m_value == newValue)
        return;

    m_value = newValue;

    if (IsInitialized())
        DoSetValue();
}

const wxString& StringParameterWidget::GetValue() const
{
    return m_value;
}

//////////////////////////////////////////////////////////////////////////

wxWindow* StringParameterWidget::CreateValueControl()
{
    m_valueItem = new wxTextCtrl(m_parameterHeader, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);

    m_valueItem->Bind(wxEVT_TEXT_ENTER, &StringParameterWidget::OnSetParameter, this);

    return m_valueItem;
}

void StringParameterWidget::InitializeParameter()
{
    DoSetValue();
}

void StringParameterWidget::UpdateImpl(const std::shared_ptr<ParameterInfo>& param)
{
    SetValue(param->getValue<std::string>());
}

void StringParameterWidget::RWModeImpl()
{
    m_valueItem->SetEditable(true);
}

void StringParameterWidget::ReadOnlyModeImpl()
{
    m_valueItem->SetEditable(false);
}

void StringParameterWidget::DoSetValue()
{
    m_valueItem->ChangeValue(m_value);
}

void StringParameterWidget::OnSetParameter(wxCommandEvent& event)
{
    m_value = m_valueItem->GetValue();

    // Let the parent know the user set the parameter value
    ParameterChangedEvent evt(GUI_SET_PARAM_VALUE, m_widgetPanel->GetId());
    evt.SetParameterId(GetParameterId());
    evt.SetCategoryId(GetCategoryId());
    evt.SetValue(GetValue());

    StoreLatestSetParamEvent(evt, -1);
}