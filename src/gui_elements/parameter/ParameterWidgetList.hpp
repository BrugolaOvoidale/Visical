#pragma once
#include <gui_elements/base_widget/BaseWidgetList.hpp>


// Forward declarations
class ParameterInfo;
class ParameterWidget;


/**
 * @brief A scrollable UI container that manages and displays a virtual list of dynamic ParameterWidgets.
 */
class ParameterWidgetList : public BaseWidgetList
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    /**
     * @brief Constructs a parameter widget list panel.
     * @param parent Pointer to the parent wxWindow workspace.
     * @param winid Unique window identifier.
     * @param pos Window position coordinates.
     * @param size Window dimensions size window.
     * @param style Window structural flags style.
     * @param name Window reference name.
     */
    ParameterWidgetList(
        wxWindow* parent,
        wxWindowID winid = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxVSCROLL | wxSIMPLE_BORDER,
        const wxString& name = wxASCII_STR(wxPanelNameStr)
    );

    ~ParameterWidgetList() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------


    /**
     * @brief Populates the container panel with a new sequence of parameters.
     *
     * This clears all existing child widgets completely prior to rendering the new list.
     *
     * @param param Collection array containing shared pointers to the data definitions.
     */
    void SetParameters(const std::vector<std::shared_ptr<ParameterInfo>>& param);

    /**
     * @brief Retrieves a parameter widget instance by its unique identifier string.
     * @param parameterId The unique string identifier of the target parameter widget.
     * @param categoryId The unique string identifier of the target parameter widget category.
     * @return A shared pointer to the requested ParameterWidget, or nullptr if not found.
	 */
    std::shared_ptr<ParameterWidget> GetWidget(
        const wxString& parameterId,
        const wxString& categoryId
    );

    /**
     * @brief Erases a parameter widget out of the list view tracking.
     * @param parameterId The unique string identifier of the item to delete.
     * @param categoryId The unique string identifier of the target parameter widget category.
     */
    void RemoveParameter(
        const wxString& parameterId,
        const wxString& categoryId
    );

    /**
     * @brief Focuses or highlights a target parameter control inside the list view structure.
     * @param parameterId The target identifier to highlight.
     * @param categoryId The unique string identifier of the target parameter widget category.
     * @return true if the requested item was found and successfully focused, false otherwise.
     */
    bool SelectParameter(
        const wxString& parameterId,
        const wxString& categoryId
    );

private:
	// Factory method creates a new parameter widget instance.
    std::shared_ptr<ParameterWidget> CreateWidget(const std::shared_ptr<ParameterInfo>& param);

    // Virtual implementation override called when a generic widget is destroyed.
    void RemoveSelectedWidgetImpl(const std::shared_ptr<BaseWidget>& baseWidget) override;

    // Virtual implementation override cleaning all tracked bidirectional items from storage.
    void RemoveAllWidgetsImpl() override;

private:
    // Bidirectional shortcut mapping parameter identity directly onto target elements.
    std::unordered_map<wxString, std::unordered_map<wxString, std::shared_ptr<ParameterWidget>>> m_parameterIdtoItem;

    // Reverse proxy lookup parsing element references into identifier keys.
    std::unordered_map<wxString, std::unordered_map<std::shared_ptr<ParameterWidget>, wxString>> m_itemToParameterId;
};