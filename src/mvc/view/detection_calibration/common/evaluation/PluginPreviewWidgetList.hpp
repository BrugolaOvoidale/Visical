#pragma once
#include <gui_elements/base_widget/BaseWidgetList.hpp>


// Forward declarations
class PluginPreviewWidget;
class PluginContextInfo;


/**
 * @brief A scrollable UI container that manages and displays a virtual list of unique EvaluationPlugins.
 */
class PluginPreviewWidgetList : public BaseWidgetList
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    PluginPreviewWidgetList(
        wxWindow* parent,
        wxWindowID winid = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxVSCROLL | wxSIMPLE_BORDER,
        const wxString& name = wxASCII_STR(wxPanelNameStr)
    );

    ~PluginPreviewWidgetList() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /**
     * @brief Clears current items and initializes the list with a new batch of plugins.
     * @param plugins A collection of plugin contexts to create widgets from.
     */
    void SetPlugins(const std::vector<std::shared_ptr<PluginContextInfo>>& plugins);

    /**
     * @brief Retrieves a plugin widget instance by its unique identifier string.
     * @param pluginId The unique string identifier of the target plugin widget.
     * @return A shared pointer to the requested PluginPreviewWidget, or nullptr if not found.
     */
    std::shared_ptr<PluginPreviewWidget> GetWidget(const wxString& pluginId);

    /**
     * @brief Gathers all currently managed plugin widgets.
     * @return A vector of shared pointers containing all active plugin widgets.
     */
    std::vector<std::shared_ptr<PluginPreviewWidget>> GetAllWidgets();

    /**
     * @brief Instantiates and appends a new plugin UI widget to the container.
     * @param pluginCtx Context information used to build the plugin widget.
     */
    void AppendPlugin(const std::shared_ptr<PluginContextInfo>& pluginCtx);

    /**
     * @brief Removes a specific plugin widget from the UI list and destroys its mapping.
     * @param toRemoveId The unique string identifier of the plugin to remove.
     */
    void RemovePlugin(const wxString& toRemoveId);

private:
    std::shared_ptr<PluginPreviewWidget> CreateWidget(const std::shared_ptr<PluginContextInfo>& plugin);

    void RemoveSelectedWidgetImpl(const std::shared_ptr<BaseWidget>& baseWidget) override;

    void RemoveAllWidgetsImpl() override;

private:
    std::unordered_map<wxString, std::shared_ptr<PluginPreviewWidget>> m_pluginIdtoItem;

    std::unordered_map<std::shared_ptr<PluginPreviewWidget>, wxString> m_itemToPluginId;
};