#pragma once
#include "ParameterWidget.hpp"


// Forward declarations
class ParameterBoolInfo;
class wxCheckBox;


/**
 * @brief A GUI widget for displaying and editing string-based parameters.
 */
class BoolParameterWidget : public ParameterWidget
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    /**
     * @brief Constructs a new String Parameter Widget.
     * @param parent Pointer to the parent window managing this control.
     * @param param Shared pointer containing the string parameter.
     */
    BoolParameterWidget(
        wxWindow* parent,
        const std::shared_ptr<ParameterBoolInfo>& param
    );

    ~BoolParameterWidget() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------
    /**
     * @brief Programmatically changes the widget's internal value and updates the UI check box.
     * @param value The new boolean state to apply.
     */
    void SetValue(bool value);

    /**
     * @brief Retrieves the current state of the parameter from the UI control.
     * @return true if checked, false otherwise.
     */
    bool GetValue() const;

private:
    wxWindow* CreateValueControl() override;

    void InitializeParameter() override;

    void UpdateImpl(const std::shared_ptr<ParameterInfo>& param) override;

    void RWModeImpl() override;

    void ReadOnlyModeImpl() override;

    void DoSetValue() override;


    //-----------------------------------------------------------------------------
    // Events
    //-----------------------------------------------------------------------------

    void OnSetParameter(wxCommandEvent& event) override;

private:
    wxCheckBox* m_valueItem{ nullptr };

    bool m_value{ false };
};