#pragma once
#include "ParameterWidget.hpp"


// Forward declarations
class ParameterStringInfo;
class wxTextCtrl;


/**
 * @brief A GUI widget for displaying and editing string-based parameters.
 */
class StringParameterWidget : public ParameterWidget
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
    StringParameterWidget(
        wxWindow* parent,
        const std::shared_ptr<ParameterStringInfo>& param
    );

    ~StringParameterWidget() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /**
     * @brief Programmatically updates the text displayed within the widget.
     * @param value The new string value to display.
     */
    void SetValue(const wxString& value);

    /**
     * @brief Retrieves the current text entered in the widget.
     * @return A constant reference to the underlying wxString value.
     */
    const wxString& GetValue() const;

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
    // Pointer to the internal wxWidgets text block interface.
    wxTextCtrl* m_valueItem{ nullptr };

	// The current string value being displayed and edited by this widget.
    wxString m_value;
};