#pragma once
#include <memory>
#include <wx/panel.h>


// Forward declarations
class wxBitmapButton;
class wxButton;
class wxSpinCtrl;
class EvaluationPluginWidgetList;
class EvaluationPanelConfigFrame;
class PluginContextInfo;
class PluginResult;


/**
 * @class EvaluationPanel
 * @brief A custom wxPanel designed to manage, display, and configure evaluation pipelines.
 *
 * This panel is split layout-wise and logically into two main scopes defined by PluginLocation:
 * 
 * 1. Per-Board: Configurations/Plugins operating on single boards.
 * 
 * 2. Per-Sequence: Configurations/Plugins operating across multiple sequential items.
 */
class EvaluationPanel : public wxPanel
{
public:
    //-----------------------------------------------------------------------------
    // Enums
    //-----------------------------------------------------------------------------

    /**
     * @enum PluginLocation
     * @brief Context scope identifier specifying where a plugin is applied within the UI.
     */
    enum class PluginLocation {
        PER_BOARD,      ///< Plugin is assigned to the active individual circuit board scope.
        PER_SEQUENCE    ///< Plugin is assigned to the running timeline/sequence scope.
    };


    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    EvaluationPanel(
        wxWindow* parent,
        wxWindowID winid = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxTAB_TRAVERSAL | wxNO_BORDER,
        const wxString& name = wxASCII_STR(wxPanelNameStr)
    );

	~EvaluationPanel() = default;

    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /** @brief Checks if the "Show Debugging Plugin Output" toggle or radio is currently selected. */
    bool IsShowDebPluginSelected() const;

    /**
     * @brief Selects and highlights a specific plugin within a target scope list.
     * @param pluginId Unique identifier of the plugin.
     * @param where Target list scope (Board or Sequence).
     * @return true if found and selected successfully.
     */
    bool SetSelectedDebPlugin(const wxString& pluginId, PluginLocation where);

    /**
     * @brief Marks a specific configuration parameter inside a plugin as modified ("dirty").
     * @param pluginId Unique identifier of the plugin.
     * @param paramId Unique identifier of the parameter.
     * @param isDirty Flag state to assign.
     */
    void MarkPluginParameterAsDirty(
        const wxString& pluginId,
        const wxString& paramId,
        bool isDirty
    );

    /**
     * @brief Retrieves the ID and location scope of the currently selected debug plugin.
     * @return An optional pair containing the Plugin ID string and its PluginLocation if selected,
     * otherwise std::nullopt.
     */
    std::optional<std::pair<wxString, PluginLocation>> GetSelectedDebPlugin() const;

    /**
     * @brief Enables or disables execution/interaction for a specific plugin.
     * @param pluginId Unique identifier of the plugin.
     * @param enable If true, activates the plugin; if false, deactivates it.
     */
    void EnablePlugin(
        const wxString& pluginId,
        bool enable = true
    );

    /** @brief Explicit shortcut to disable a plugin interface. @see EnablePlugin */
    void DisablePlugin(const wxString& pluginId) { EnablePlugin(pluginId, false); }

    /**
     * @brief Re-renders or updates the contextual presentation metadata of a plugin entry.
     * @param pluginCtx Shared pointer holding current state and configuration details.
     */
    void UpdatePlugin(const std::shared_ptr<PluginContextInfo>& pluginCtx);

    /**
     * @brief Dispatches calculation results (e.g., pass/fail indicators) to a plugin's UI view row.
     * @param pluginResult Shared pointer containing algorithmic analysis output.
     */
    void UpdatePluginResult(const std::shared_ptr<PluginResult>& pluginResult);

    /**
     * @brief Uniformly modifies the alert/warning threshold entry for all active elements in a scope.
     * @param threshold Floating point value threshold.
     * @param where Target list scope (Board or Sequence).
     */
    void UpdateAllPluginsThreshold(
        double threshold,
        PluginLocation where
    );

    /**
     * @brief Appends a new plugin row/item representation into the designated UI listing.
     * @param pluginCtx Target plugin metadata context block.
     * @param where Target list scope defining which sub-view list receives the element.
     */
    void AppendPlugin(
        const std::shared_ptr<PluginContextInfo>& pluginCtx,
        PluginLocation where
    );

    /** @brief Globally activates preview processing states for all items in a specified scope list. */
    void EnableAllPluginsPreview(PluginLocation where);

    /** @brief Activates the live visualization/preview processing flag for an individual plugin. */
    void EnablePluginPreview(const wxString& pluginId);

    /** @brief Disables live visualization/preview processing for an individual plugin in a designated scope. */
    void DisablePluginPreview(
        const wxString& pluginId,
        PluginLocation where
    );

    /**
     * @brief Resets and overrides the current batch view selection list with a fresh vector collection.
     * @param plugins Collection of updated plugin context pointers.
     * @param where Target layout area list selection.
     */
    void SetPluginsPreview(
        const std::vector<std::shared_ptr<PluginContextInfo>>& plugins,
        EvaluationPanel::PluginLocation where
    );

    /** @brief Resets visual output fields inside a specified plugin row. */
    void ClearPlugin(const wxString& toClearId);

    /** @brief Resets visual components for all elements in a given layout scope. */
    void ClearAllPlugins(PluginLocation where);

    /** @brief Erases and deconstructs a plugin widget entry row out of the user interface by its string ID. */
    void RemovePlugin(const wxString& toRemoveId);

    /** @brief Wipes out all plugin rows inside a designated UI category block list. */
    void RemoveAllPlugins(PluginLocation where);

private:

    //-----------------------------------------------------------------------------
    // Events
    //-----------------------------------------------------------------------------

    void OnBoardAddPlugin(wxCommandEvent& event);

    void OnBoardRemovePlugin(wxCommandEvent& event);

    void OnBoardOpenParamsFrame(wxCommandEvent& event);

    void OnBoardOpenConfigFrame(wxCommandEvent& event);

    void OnBoardSelectPlugin(wxCommandEvent& event);

    void OnBoardShowDebResult(wxCommandEvent& event);

    void OnBoardLoadPlugins(wxCommandEvent& event);

    void OnBoardSavePlugins(wxCommandEvent& event);

    void OnBoardGlobalThr(wxCommandEvent& event);


    void OnSequenceAddPlugin(wxCommandEvent& event);

    void OnSequenceRemovePlugin(wxCommandEvent& event);

    void OnSequenceOpenParamsFrame(wxCommandEvent& event);

    void OnSequenceOpenConfigFrame(wxCommandEvent& event);

    void OnSequenceSelectPlugin(wxCommandEvent& event);

    void OnSequenceShowDebResult(wxCommandEvent& event);

    void OnSequenceLoadPlugins(wxCommandEvent& event);

    void OnSequenceSavePlugins(wxCommandEvent& event);

    void OnSequenceGlobalThr(wxCommandEvent& event);

private:
    wxBitmapButton* m_loadBoardPlugins;

    wxBitmapButton* m_saveBoardPlugins;

    wxBitmapButton* m_configBoardPlugins;

    wxButton* m_applyGlobalBoardThr;

    wxSpinCtrl* m_globalBoardThr;

    EvaluationPluginWidgetList* m_boardPluginsList;

    EvaluationPanelConfigFrame* m_boardConfigFrame;


    wxBitmapButton* m_loadBoardSeqPlugins;

    wxBitmapButton* m_saveBoardSeqPlugins;

    wxBitmapButton* m_configBoardSeqPlugins;

    wxButton* m_applyGlobalBoardSeqThr;

    wxSpinCtrl* m_globalBoardSeqThr;

    EvaluationPluginWidgetList* m_boardSequencePluginsList;

	EvaluationPanelConfigFrame* m_sequenceConfigFrame;
};