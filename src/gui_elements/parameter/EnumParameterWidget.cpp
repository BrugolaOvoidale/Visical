#include "EnumParameterWidget.hpp"
#include <wx/panel.h>
#include <wx/choice.h>
#include <parameter/enum/ParameterEnumInfo.hpp>
#include "ParameterWidgetEvents.hpp"


EnumParameterWidget::EnumParameterWidget(
    wxWindow* parent,
    const std::shared_ptr<ParameterEnumInfo>& param)
    : ParameterWidget(
        parent,
        param)
{
    const auto& int_choices = param->getPossibleIntValues();
    std::unordered_map<int, wxString> choices;

    for (const auto& [key, value] : int_choices) choices.emplace(key, value);

    SetChoices(std::move(choices));

    SetSelection(param->getStrValue());
}

//////////////////////////////////////////////////////////////////////////

void EnumParameterWidget::SetSelection(const wxString& newSelection)
{
    if (m_value == newSelection)
        return;

	if (!m_possibleStrValues.contains(newSelection))
        return;

    m_value = newSelection;

    if (IsInitialized())
        DoSetValue();
}

void EnumParameterWidget::SetSelection(int newSelection)
{
    auto it = m_possibleIntValues.find(newSelection);
    if (it == m_possibleIntValues.end())
        return;

	m_value = it->second;

    if (IsInitialized())
        DoSetValue();
}

const wxString& EnumParameterWidget::GetSelection() const
{
    return m_value;
}

void EnumParameterWidget::SetChoices(std::unordered_map<int, wxString> possibleValues)
{
    if (m_possibleIntValues == possibleValues)
        return;

    m_possibleIntValues = std::move(possibleValues);

    m_possibleStrValues.clear();
    for (const auto& [key, value] : m_possibleIntValues)
    {
        m_possibleStrValues[value] = key;
    }

    if (IsInitialized())
        DoSetChoices();
}

//////////////////////////////////////////////////////////////////////////

wxWindow* EnumParameterWidget::CreateValueControl()
{
    m_valueItem = new wxChoice(m_parameterHeader, wxID_ANY);

    m_valueItem->Bind(wxEVT_CHOICE, &EnumParameterWidget::OnSetParameter, this);

    return m_valueItem;
}

void EnumParameterWidget::InitializeParameter()
{
    DoSetChoices();

    DoSetValue();
}

void EnumParameterWidget::UpdateImpl(const std::shared_ptr<ParameterInfo>& param)
{
    std::shared_ptr<ParameterEnumInfo> enumParam = std::dynamic_pointer_cast<ParameterEnumInfo>(param);

    const auto& int_choices = enumParam->getPossibleIntValues();
    std::unordered_map<int, wxString> choices;
    for (const auto& [key, value] : int_choices) choices.emplace(key, value);

    SetChoices(std::move(choices));

    SetSelection(enumParam->getStrValue());
}

void EnumParameterWidget::DoSetValue()
{
    m_valueItem->SetStringSelection(m_value);
}

void EnumParameterWidget::DoSetChoices()
{
    m_valueItem->Clear();

    for (const auto& [key, value] : m_possibleIntValues)
    {
        m_valueItem->AppendString(value);
    }

    DoSetValue();
}

void EnumParameterWidget::RWModeImpl()
{
	if (IsInitialized())
        m_valueItem->Enable();
}

void EnumParameterWidget::ReadOnlyModeImpl()
{
    if (IsInitialized())
        m_valueItem->Disable();
}

void EnumParameterWidget::OnSetParameter(wxCommandEvent& event)
{
    m_value = m_valueItem->GetStringSelection();

    // Let the parent know this board was clicked
    ParameterChangedEvent evt(GUI_SET_PARAM_VALUE, m_widgetPanel->GetId());
    evt.SetParameterId(GetParameterId());
    evt.SetCategoryId(GetCategoryId());
    evt.SetValue(GetSelection());

    StoreLatestSetParamEvent(evt, -1);
}
