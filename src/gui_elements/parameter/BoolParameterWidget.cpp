#include "BoolParameterWidget.hpp"
#include <wx/panel.h>
#include <wx/checkbox.h>
#include <parameter/bool/ParameterBoolInfo.hpp>
#include "ParameterWidgetEvents.hpp"


BoolParameterWidget::BoolParameterWidget(
    wxWindow* parent,
    const std::shared_ptr<ParameterBoolInfo>& param)
    : ParameterWidget(
        parent, 
        param)
{
    SetValue(param->getValue());
}

//////////////////////////////////////////////////////////////////////////

void BoolParameterWidget::SetValue(bool newValue)
{
    if (m_value == newValue)
        return;

    m_value = newValue;

    if (IsInitialized())
        DoSetValue();

}

bool BoolParameterWidget::GetValue() const
{
    return m_value;
}

//////////////////////////////////////////////////////////////////////////

wxWindow* BoolParameterWidget::CreateValueControl()
{
    m_valueItem = new wxCheckBox(m_parameterHeader, wxID_ANY, wxEmptyString);

    m_valueItem->Bind(wxEVT_CHECKBOX, &BoolParameterWidget::OnSetParameter, this);

    return m_valueItem;
}

void BoolParameterWidget::InitializeParameter()
{
    DoSetAccessMode();

    DoSetValue();
}

void BoolParameterWidget::UpdateImpl(const std::shared_ptr<ParameterInfo>& param)
{
    SetValue(param->getValue<bool>());
}

void BoolParameterWidget::RWModeImpl()
{
    if (IsInitialized())
        m_valueItem->Enable();
}

void BoolParameterWidget::ReadOnlyModeImpl()
{
    if (IsInitialized())
        m_valueItem->Disable();
}

void BoolParameterWidget::DoSetValue()
{
    m_valueItem->SetValue(m_value);
}

void BoolParameterWidget::OnSetParameter(wxCommandEvent& event)
{
    m_value = m_valueItem->GetValue();

    // Let the parent know this parameter changed value
    ParameterChangedEvent evt(GUI_SET_PARAM_VALUE, m_widgetPanel->GetId());
    evt.SetParameterId(GetParameterId());
    evt.SetCategoryId(GetCategoryId());
    evt.SetValue(GetValue());


    StoreLatestSetParamEvent(evt, 300);
}

