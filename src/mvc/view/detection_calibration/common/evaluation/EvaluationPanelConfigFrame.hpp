#pragma once
#include <memory>
#include <wx/frame.h>


// Forward declarations
class wxPanel;
class PluginContextInfo;
class PluginPreviewWidgetList;


/**
 * @brief A frame window dedicated to displaying and configuring available evaluation plugins.
 */
class EvaluationPanelConfigFrame : public wxFrame
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    EvaluationPanelConfigFrame(
        wxWindow* parent,
        const wxString& title
    );

    ~EvaluationPanelConfigFrame() override = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /**
     * @brief Shows the frame and brings it to the front.
     */
    void Open();

    /**
     * @brief Populates the frame with a fresh list of plugin preview widgets.
     * @param plugins A collection of shared pointers to PluginContextInfo structures.
     */
    void SetPlugins(const std::vector<std::shared_ptr<PluginContextInfo>>& plugins);

    /**
     * @brief Toggles the visual preview state to 'enabled' for all registered plugins.
     */
    void EnableAllPluginsPreview();

    /**
     * @brief Enables the visual preview for a specific plugin.
     * @param pluginId The unique string identifier of the target plugin.
     */
    void EnablePluginPreview(const wxString& pluginId);

    /**
     * @brief Disables the visual preview for a specific plugin.
     * @param pluginId The unique string identifier of the target plugin.
     */
    void DisablePluginPreview(const wxString& pluginId);

private:
    void BuildUI();

    void OnClose(wxCloseEvent& event);

private:
	PluginPreviewWidgetList* m_pluginPreviewWidgetList;
};
