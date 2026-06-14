#pragma once
#include "ParameterWidget.hpp"


// Forward declarations
class wxButton;


/**
 * @brief A GUI widget for displaying command-based parameters.
 */
class CommandParameterWidget : public ParameterWidget
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    /**
     * @brief Constructs a new Command Parameter Widget.
     * @param parent Pointer to the parent window managing this control.
     * @param param Shared pointer containing the parameter.
     */
    CommandParameterWidget(
        wxWindow* parent,
        const std::shared_ptr<ParameterInfo>& param
    );

    ~CommandParameterWidget() = default;


private:
    wxWindow* CreateValueControl() override;
    
	void InitializeParameter() override;

	void UpdateImpl(const std::shared_ptr<ParameterInfo>& param) override;

	void DoSetValue() override;

    void RWModeImpl() override;

    void ReadOnlyModeImpl() override;


    //-----------------------------------------------------------------------------
    // Events
    //-----------------------------------------------------------------------------

    void OnSetParameter(wxCommandEvent& event) override;

private:
    wxButton* m_value{ nullptr };
};