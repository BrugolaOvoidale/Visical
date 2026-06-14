#pragma once
#include "NumericParameterWidget.hpp"


template<typename T>
inline NumericParameterWidget<T>::NumericParameterWidget(
    wxWindow* parent,
    const std::shared_ptr<ParameterNumericInfo<T>>& param)
    : ParameterWidget(parent, param)
{
	T value = param->getValue();
    T minValue = value + GetDefaultMin();
    T maxValue = value + GetDefaultMax();
    T increment = GetDefaultIncrement();

    if (param->getMin().has_value()) minValue = param->getMin().value();
    if (param->getMax().has_value()) maxValue = param->getMax().value();
    if (param->getIncrement().has_value()) increment = param->getIncrement().value();

    SetRange(minValue, maxValue);
    SetIncrement(increment);
    ConfigureDigits(increment);
    SetValue(param->getValue());
    SetInsertionPoint();
}

/////////////////////////////////////////////////////////

template<typename T>
inline void NumericParameterWidget<T>::SetValue(T newValue)
{
    if (m_value == newValue)
        return;

    m_value = newValue;

    if (IsInitialized())
        DoSetValue();
}

template<typename T>
inline T NumericParameterWidget<T>::GetValue() const
{
    return m_value;
}

template<typename T>
inline void NumericParameterWidget<T>::SetRangeMin(T newMin)
{
    if (m_minValue == newMin)
        return;

    m_minValue = newMin;

    if (IsInitialized())
        DoSetRange();
}

template<typename T>
inline void NumericParameterWidget<T>::SetRangeMax(T newMax)
{
    if (m_maxValue == newMax)
        return;

    m_maxValue = newMax;

    if (IsInitialized())
        DoSetRange();
}

template<typename T>
inline void NumericParameterWidget<T>::SetRange(T newMin, T newMax)
{
    if (m_minValue == newMin && m_maxValue == newMax)
        return;

    m_minValue = newMin;
    m_maxValue = newMax;

    if (IsInitialized())
        DoSetRange();
}

template<typename T>
inline void NumericParameterWidget<T>::SetIncrement(T newInc)
{
    if (m_increment == newInc)
		return;

    m_increment = newInc;

    if (m_increment == T(0))
    {
        m_increment = GetDefaultIncrement();
    }

    if(IsInitialized())
		DoSetIncrement();
}

/////////////////////////////////////////////////////////

template<typename T>
inline wxWindow* NumericParameterWidget<T>::CreateValueControl()
{
    wxArrayString allowed;
    allowed.Add("0"); allowed.Add("1"); allowed.Add("2"); allowed.Add("3");
    allowed.Add("4"); allowed.Add("5"); allowed.Add("6"); allowed.Add("7");
    allowed.Add("8"); allowed.Add("9");
    allowed.Add("-");  // allow negative sign

    if constexpr (std::is_same_v<T, double>)
    {
        allowed.Add(".");  // allow decimal point for double
    }

    wxTextValidator validator(wxFILTER_INCLUDE_CHAR_LIST);
    validator.SetIncludes(allowed);

    m_valueItem = new wxSpinCtrlDouble(m_parameterHeader, wxID_ANY,
        wxEmptyString, wxDefaultPosition, wxDefaultSize,
        wxTE_PROCESS_ENTER | wxSP_ARROW_KEYS);

    m_valueItem->SetValidator(validator);

    m_valueItem->Bind(wxEVT_SPINCTRLDOUBLE, &NumericParameterWidget::OnSetParameter, this);
    m_valueItem->Bind(wxEVT_TEXT_ENTER, &NumericParameterWidget::OnSetParameter, this);

    BindSelectable(m_valueItem, true);

    return m_valueItem;
}

template<typename T>
inline void NumericParameterWidget<T>::InitializeParameter()
{
    DoSetRange();

    DoSetIncrement();

    DoConfigureDigits();

    DoSetValue();

    DoSetInsertionPoint();
}

template<typename T>
inline void NumericParameterWidget<T>::UpdateImpl(const std::shared_ptr<ParameterInfo>& param)
{
    std::shared_ptr<ParameterNumericInfo<T>> numericParam =
        std::dynamic_pointer_cast<ParameterNumericInfo<T>>(param);

    T value = numericParam->getValue();
    T minValue = value + GetDefaultMin();
    T maxValue = value + GetDefaultMax();
    T increment = GetDefaultIncrement();

    if (numericParam->getMin().has_value()) minValue = numericParam->getMin().value();
    if (numericParam->getMax().has_value()) maxValue = numericParam->getMax().value();
    if (numericParam->getIncrement().has_value()) increment = numericParam->getIncrement().value();

    SetRange(minValue, maxValue);
    SetIncrement(increment);
    ConfigureDigits(increment);
    SetValue(value);
    SetInsertionPoint();
}

template<typename T>
inline void NumericParameterWidget<T>::DoSetValue()
{
    m_valueItem->SetValue(m_value);
}

template<typename T>
inline void NumericParameterWidget<T>::DoSetRange()
{
    m_valueItem->SetRange(m_minValue, m_maxValue);
}

template<typename T>
inline void NumericParameterWidget<T>::DoSetIncrement()
{
    m_valueItem->SetIncrement(m_increment);
}

template<typename T>
inline void NumericParameterWidget<T>::RWModeImpl()
{
#ifdef __WXMSW__
    m_valueItem->GetText()->SetEditable(true);
    m_valueItem->GetSpinButton()->Enable();
    m_valueItem->GetSpinButton()->Show();
#else
    m_valueItem->Enable();
#endif
}

template<typename T>
inline void NumericParameterWidget<T>::ReadOnlyModeImpl()
{
#ifdef __WXMSW__
    m_valueItem->GetText()->SetEditable(false);
    m_valueItem->GetSpinButton()->Disable();
    m_valueItem->GetSpinButton()->Hide();
#else
    m_valueItem->Disable();
#endif
}

template<typename T>
inline void NumericParameterWidget<T>::ConfigureDigits(T increment)
{
    if (!IsInitialized())
        return;

    DoConfigureDigits();
}

template<typename T>
inline void NumericParameterWidget<T>::DoConfigureDigits()
{
    if constexpr (std::is_same_v<T, double>)
    {
        unsigned int digits = 0;
        double tempInc = m_increment;
        while (tempInc < 1.0 && digits < 10)
        {
            tempInc *= 10;
            digits++;
        }
        m_valueItem->SetDigits(digits);
    }
    // For int, no digits to configure
}

template<typename T>
inline void NumericParameterWidget<T>::SetInsertionPoint()
{
    if (!IsInitialized())
        return;

    DoSetInsertionPoint();
}

template<typename T>
inline void NumericParameterWidget<T>::DoSetInsertionPoint()
{
#ifdef __WXMSW__
    if (wxTextCtrl* txt = m_valueItem->GetText())
    {
        txt->SetInsertionPoint(0);
    }
#endif
}

template<typename T>
inline constexpr T NumericParameterWidget<T>::GetDefaultMin()
{
    if constexpr (std::is_same_v<T, double>)
        return -9999.9;
    else
        return -9999;
}

template<typename T>
inline constexpr T NumericParameterWidget<T>::GetDefaultMax()
{
    if constexpr (std::is_same_v<T, double>)
        return 9999.9;
    else
        return 9999;
}

template<typename T>
inline constexpr T NumericParameterWidget<T>::GetDefaultIncrement()
{
    if constexpr (std::is_same_v<T, double>)
        return 0.1;
    else
        return 1;
}

template<typename T>
inline void NumericParameterWidget<T>::OnSetParameter(wxCommandEvent& event)
{
    int debounceTime = -1;

    if (event.GetEventType() == wxEVT_TEXT_ENTER)
    {
        m_handledEnter = true;

        const wxString valueStr = event.GetString();
        
        if constexpr (std::is_same_v<T, int>)
        {
            int valueInt;
            if (valueStr.ToInt(&valueInt))
                m_value = valueInt;
        }
        else
        {
            double valueDouble;
            if (valueStr.ToDouble(&valueDouble))
                m_value = valueDouble;
        }
    }
    else if (event.GetEventType() == wxEVT_SPINCTRLDOUBLE)
    {
        if (m_handledEnter)
        {
            m_handledEnter = false;
            return; // skip duplicate
        }

        m_value = m_valueItem->GetValue();

        debounceTime = 800;
    }

    ParameterChangedEvent evt(GUI_SET_PARAM_VALUE, m_widgetPanel->GetId());
    evt.SetParameterId(GetParameterId());
    evt.SetCategoryId(GetCategoryId());
    evt.SetValue(m_value);

    StoreLatestSetParamEvent(evt, debounceTime);
}