#pragma once
#include <gui_elements/base_widget/BaseWidgetList.hpp>


// Forward declarations
class EvaluationPluginWidget;
class PluginContextInfo;


/**
 * @brief A scrollable UI container that manages and displays a virtual list of unique EvaluationPlugins.
 */
class EvaluationPluginWidgetList : public BaseWidgetList
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    EvaluationPluginWidgetList(
        wxWindow* parent,
        wxWindowID winid = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxVSCROLL | wxSIMPLE_BORDER,
        const wxString& name = wxASCII_STR(wxPanelNameStr)
    );

    ~EvaluationPluginWidgetList() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /**
     * @brief Instantiates and appends a new plugin UI widget to the container.
     * @param pluginCtx Context information used to build the plugin widget.
     */
    void AppendPlugin(const std::shared_ptr<PluginContextInfo>& pluginCtx);

    /**
     * @brief Retrieves a plugin widget instance by its unique identifier string.
     * @param pluginId The unique string identifier of the target plugin widget.
     * @return A shared pointer to the requested EvaluationPluginWidget, or nullptr if not found.
     */
    std::shared_ptr<EvaluationPluginWidget> GetWidget(const wxString& pluginId);

    /**
     * @brief Gathers all currently managed plugin widgets.
     * @return A vector of shared pointers containing all active plugin widgets.
     */
    std::vector<std::shared_ptr<EvaluationPluginWidget>> GetAllWidgets();

    /**
     * @brief Removes a specific plugin widget from the UI list and destroys its mapping.
     * @param toRemoveId The unique string identifier of the plugin to remove.
     */
    void RemovePlugin(const wxString& toRemoveId);

    /**
     * @brief Gets the ID of the plugin that currently has its "Show Debug" button active.
     * @return An optional containing the active plugin ID string, or std::nullopt if none is active.
     */
    std::optional<wxString> GetSelectedDebPlugin() const;

    /**
     * @brief Checks if *any* plugin's "Show Debug" feature is currently activated.
     * @return true if a debug plugin is selected, false otherwise.
     */
    bool IsShowDebPluginSelected() const;

    /**
     * @brief Forces the UI to visually check/activate the debug button of a specific plugin.
     * @param pluginId The unique ID of the target plugin.
     * @return true if the plugin was found and checked successfully, false otherwise.
     */
    bool CheckDebButton(const wxString& pluginId);

    /**
     * @brief Programmatically unchecks the currently active "Show Debug" button across the entire list.
     */
    void UncheckDebButton();

    /**
     * @brief Closes the configuration or parameter sub-frame that was last opened by a plugin.
     */
    void CloseLastParametersFrame();

private:
    void RemoveSelectedWidgetImpl(const std::shared_ptr<BaseWidget>& baseWidget) override;

    void RemoveAllWidgetsImpl() override;

    void OnShowDebugResult(wxCommandEvent& event);

    void OnOpenPluginParameters(wxCommandEvent& event);

private:
    std::shared_ptr<EvaluationPluginWidget> m_lastShowDebPlugin{ nullptr };

    std::shared_ptr<EvaluationPluginWidget> m_lastParamsFrame{ nullptr };

    std::unordered_map<wxString, std::shared_ptr<EvaluationPluginWidget>> m_pluginIdtoItem;

    std::unordered_map<std::shared_ptr<EvaluationPluginWidget>, wxString> m_itemToPluginId;
};