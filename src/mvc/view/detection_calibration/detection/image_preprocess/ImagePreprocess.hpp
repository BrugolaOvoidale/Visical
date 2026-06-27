#pragma once
#include <memory>
#include <wx/frame.h>
#include "../ParameterLocation.hpp"


// Forward declarationsì
class ParameterWidgetList;
class ParameterInfo;
class wxPanel;


/**
 * @brief Window frame responsible for configuring and displaying image preprocessing parameters.
 */
class ImagePreprocess : public wxFrame
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    ImagePreprocess(
        wxWindow* parent,
        const wxString& title
    );

    ~ImagePreprocess() override = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /**
     * @brief Shows the frame and brings it to the front.
     */
    void Open();

    /**
     * @brief Populates the frame with a collection of parameters.
     * @param params A list of shared pointers to parameter metadata.
     * @param where Specifies the operational location context within the system.
     */
    void SetParameters(
        const std::vector<std::shared_ptr<ParameterInfo>>& params,
        DetectionView::ParameterLocation where
    );

    /**
     * @brief Refreshes the visual state of a specific parameter control.
     * @param param The updated parameter configuration to synchronize with the UI.
     */
    void UpdateParameter(const std::shared_ptr<ParameterInfo>& param);

    /**
     * @brief Marks a specific parameter to show it has unsaved changes.
     * @param paramId The unique identifier string of the target parameter.
     * @param categoryId The unique identifier string of the target parameter category.
     * @param isDirty Pass true to mark as modified/unsaved, false to clear the flag.
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
    ParameterWidgetList* m_preProcParamsList;
};
