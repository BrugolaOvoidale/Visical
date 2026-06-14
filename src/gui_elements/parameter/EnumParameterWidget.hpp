#pragma once
#include "ParameterWidget.hpp"


// Forward declarations
class ParameterEnumInfo;
class wxChoice;


/**
 * @brief A GUI widget for displaying and editing enum-based parameters.
 */
class EnumParameterWidget : public ParameterWidget
{
public:
    /**
     * @brief Constructs a new Enum Parameter Widget.
     * @param parent Pointer to the parent window managing this control.
     * @param param Shared pointer containing the enum parameter.
     */
    EnumParameterWidget(
        wxWindow* parent,
        const std::shared_ptr<ParameterEnumInfo>& param
    );

    ~EnumParameterWidget() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /**
     * @brief Programmatically changes the active selection using its string label.
     * @param selection The string matching one of the available choices.
     */
    void SetSelection(const wxString& selection);

    /**
     * @brief Programmatically changes the active selection using its underlying integer enum value.
     * @param selection The integer index/value matching one of the available choices.
     */
    void SetSelection(int selection);

    /**
     * @brief Retrieves the currently selected item's string label.
     * @return A constant reference to the active wxString selection.
     */
    const wxString& GetSelection() const;

    /**
     * @brief populates or replaces the selection choices available in the dropdown.
     * @param choices A map of enum integer values to their localized display strings.
     */
    void SetChoices(std::unordered_map<int, wxString> choices);

private:
    wxWindow* CreateValueControl() override;

	void InitializeParameter() override;

    void UpdateImpl(const std::shared_ptr<ParameterInfo>& param) override;

    void DoSetValue() override;

    void DoSetChoices();

    void RWModeImpl() override;

    void ReadOnlyModeImpl() override;


    //-----------------------------------------------------------------------------
    // Events
    //-----------------------------------------------------------------------------

    void OnSetParameter(wxCommandEvent& event) override;

private:
    wxChoice* m_valueItem{ nullptr };

    wxString m_value;

    std::unordered_map<wxString, int> m_possibleStrValues;

    std::unordered_map<int, wxString> m_possibleIntValues;
};