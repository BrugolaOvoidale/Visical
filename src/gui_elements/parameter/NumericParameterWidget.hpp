#pragma once
#include <wx/spinctrl.h>
#include <wx/valtext.h>
#include <wx/textctrl.h>
#include <wx/panel.h>
#include <parameter/numeric/ParameterNumericInfo.hpp>
#include "ParameterWidget.hpp"
#include "ParameterWidgetEvents.hpp"


/**
 * @brief A template-based wxWidgets control wrapper for displaying and editing numeric parameter values.
 *
 * @tparam T The numeric type. Explicitly constrained to 'int' or 'double' via static assertion.
 */
template<typename T>
class NumericParameterWidget : public ParameterWidget
{
    static_assert(
        std::is_same_v<T, int> || std::is_same_v<T, double>,
        "ParameterNumericWidget only supports int and double"
        );

public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    /**
     * @brief Constructs a new Numeric Parameter Widget.
     * @param parent Pointer to the parent window managing this control.
     * @param param Shared pointer containing the numeric parameter.
     */
    NumericParameterWidget(
        wxWindow* parent,
        const std::shared_ptr<ParameterNumericInfo<T>>& param
    );

    ~NumericParameterWidget() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /**
     * @brief Programmatically overrides the active numeric value shown in the widget.
     * @param newValue The value to push down to the UI control.
     */
    void SetValue(T newValue);

    /**
     * @brief Retrieves the current value typed or spun by the user.
     * @return The numeric value currently stored inside the local state buffer.
     */
    T GetValue() const;

    /** @brief Sets the absolute minimum bounding value permitted by this control. */
    void SetRangeMin(T min);

    /** @brief Sets the absolute maximum bounding value permitted by this control. */
    void SetRangeMax(T max);

    /**
     * @brief Convenience function to apply both limits simultaneously.
     * @param min The lower boundary limit.
     * @param max The upper boundary limit.
     */
    void SetRange(T min, T max);

    /**
     * @brief Updates the step size added or subtracted when clicking the up/down arrows.
     * @param inc Step increment value.
     */
    void SetIncrement(T inc);

private:
    wxWindow* CreateValueControl() override;

	void InitializeParameter() override;

    void UpdateImpl(const std::shared_ptr<ParameterInfo>& param) override;

    void DoSetValue() override;

    void DoSetRange();

    void DoSetIncrement();

    void RWModeImpl() override;

    void ReadOnlyModeImpl() override;

    void ConfigureDigits(T increment);

    void DoConfigureDigits();

    void SetInsertionPoint();

    void DoSetInsertionPoint();

    static constexpr T GetDefaultMin();

    static constexpr T GetDefaultMax();

    static constexpr T GetDefaultIncrement();

    //-----------------------------------------------------------------------------
    // Events
    //-----------------------------------------------------------------------------

    void OnSetParameter(wxCommandEvent& event) override;

private:
    wxSpinCtrlDouble* m_valueItem{ nullptr };

    T m_value;

    T m_minValue;

    T m_maxValue;

    T m_increment;

    bool m_handledEnter{ false };
};


// Type aliases

using IntParameterWidget = NumericParameterWidget<int>;
using DoubleParameterWidget = NumericParameterWidget<double>;

#include "NumericParameterWidget.inl"