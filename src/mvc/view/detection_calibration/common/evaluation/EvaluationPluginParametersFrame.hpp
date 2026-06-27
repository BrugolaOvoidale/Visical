#pragma once
#include <memory>
#include <wx/frame.h>


// Forward declarations
class wxPanel;
class ParameterInfo;
class ParameterWidgetList;


/**
 * @brief A frame window dedicated to displaying and configuring evaluation plugin parameters.
 */
class EvaluationPluginParametersFrame : public wxFrame
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    EvaluationPluginParametersFrame(
        wxWindow* parent,
        const wxString& title
    );

    ~EvaluationPluginParametersFrame() override = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /**
     * @brief Shows the frame and brings it to the front.
     */
    void Open();

    /**
     * @brief Populates the frame with a fresh list of parameter widgets.
     * @param params A collection of shared pointers to ParameterInfo structures.
     */
    void SetParameters(const std::vector<std::shared_ptr<ParameterInfo>>& params);

    /**
     * @brief Updates the visual state of a specific parameter widget without rebuilding the list.
     * @param param The parameter info containing the updated value or state.
     */
    void UpdateParameter(const std::shared_ptr<ParameterInfo>& param);

    /**
     * @brief Toggles a visual indicator to show if a parameter has unsaved changes.
     * @param paramId The unique string identifier of the parameter.
     * @param categoryId The unique string identifier of the parameter category.
     * @param isDirty Set to true if the parameter has been modified, false to clear the flag.
     */
    void MarkParameterAsDirty(
        const wxString& paramId,
        const wxString& categoryId,
        bool isDirty
    );

private:
    void BuildUI();

    void OnClose(wxCloseEvent& event);

private:
    ParameterWidgetList* m_pluginParamsList;
};
