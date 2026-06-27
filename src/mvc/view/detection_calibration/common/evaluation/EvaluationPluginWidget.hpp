#pragma once
#include <wx/timer.h>
#include <memory>
#include <gui_elements/base_widget/BaseWidget.hpp>


// Forward declarations
class wxCheckBox;
class wxSpinCtrlDouble;
class wxToggleButton;
class wxBitmapButton;
class wxTimerEvent;
class EvaluationPluginParametersFrame;
class PluginContextInfo;
class PluginResult;
class ParameterInfo;
class ParameterChangedEvent;
class ParameterEvent;


/**
 * @brief A graphic component handling the display for an individual evaluation plugin.
 */
class EvaluationPluginWidget : public BaseWidget
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    /**
     * @brief Constructs a new Evaluation Plugin Widget.
     * @param parent Pointer to the parent window layer.
     * @param camera Shared handle to structural info describing this specific plugin instance.
     */
    EvaluationPluginWidget(
        wxWindow* parent,
        const std::shared_ptr<PluginContextInfo>& pluginCtx
    );

    ~EvaluationPluginWidget() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------
    
    /** @brief Gets the unique string Id of the underlying plugin. */
    const wxString& GetPluginId() const;

    /**
     * @brief Completely refreshes the widget's internal data using a new plugin context.
     * @param pluginCtx The updated plugin context info layout.
     */
    void UpdatePlugin(const std::shared_ptr<PluginContextInfo>& pluginCtx);

    /**
     * @brief Updates the widget view with the latest evaluation execution results.
     * @param pluginRes Shared pointer containing the new score, notes, or visual debug data.
     */
    void UpdatePluginResult(const std::shared_ptr<PluginResult>& pluginRes);

    /**
     * @brief Flags a specific parameter visually if its value has unsaved changes.
     * @param paramId Unique identifier of the parameter.
     * @param categoryId Unique identifier of the parameter category.
     * @param isDirty True to show modified styling; false to revert to standard styling.
     */
    void MarkParameterAsDirty(
        const wxString& paramId,
        const wxString& categoryId,
        bool isDirty
    );

    /** @brief Toggles the processing/active checkbox state of the plugin. */
    void SetActive(bool isActive);

    /** @brief Checks if the evaluation plugin is currently marked active by the user. */
    bool IsActive() const;

    /** @brief Formats and updates the text note/label associated with the last run. */
    void SetNote(const wxString& newNote);

    /** @brief Retrieves the currently displayed evaluation status note string. */
    const wxString& GetNote() const;

    /** @brief Clears the numerical score field, setting it back to an uncomputed state. */
    void UnsetScore();

    /** @brief Sets and displays the evaluation quality score metric. */
    void SetScore(double score);

    /** @brief Retrieves the current quality score metric. Returns negative if unset. */
    double GetScore() const;

    /** @brief Programmatically toggles the status of the "Show Debug Graphics" option. */
    void SetDebugResult(bool isChecked);

    /** @brief Checks whether visual overlay debugging is toggled on. */
    bool IsDebugResultShown() const;

    /** @brief Synchronizes the numeric limits spin control with a precise trigger threshold value. */
    void SetThreshold(double threshold);

    /** @brief Retrieves the current evaluation trigger threshold value. */
    double GetThreshold() const;

    /**
     * @brief Populates the internal configurations array with controllable parameter bindings.
     * @param params A list of shared parameter descriptors.
     */
    void SetParameters(const std::vector<std::shared_ptr<ParameterInfo>>& params);

    /** @brief Toggles whether the threshold spin-control is enabled or greyed out in the interface. */
    void EnableThreshold(bool enable = true);

    /** @brief Resets all text widgets, scores, and clear data states back to blank configurations. */
    void ClearData();

    /** @brief Instantiates or raises the separate standalone parameters pop-up window frame. */
    void OpenParametersFrame();

    /** @brief Closes and cleans up the standalone parameters window frame if it is active. */
    void CloseParametersFrame();

private:
    wxPanel* CreateHeader() override;

    void InitializeImpl() override;

    void SetDescriptionImpl() override;

    void DoSetActive();

    void DoSetNote();

    void DoUnsetScore();

    void DoSetScore();

    void DoSetDebugResult();

    void DoSetThreshold();

    void DoSetParameters();

    void DoEnableThreshold();


    //-----------------------------------------------------------------------------
    // Events
    //-----------------------------------------------------------------------------

    void OnSpecializedWidgetClick(wxMouseEvent& event) override;

    void OnEnable(wxCommandEvent& event);

    void OnOpenPluginParametersFrame(wxCommandEvent& event);

    void OnShowDebugResult(wxCommandEvent& event);

    void OnChangeThreshold(wxCommandEvent& event);

    void OnSetPluginParameter(ParameterChangedEvent& event);

    void OnResetPluginParameter(ParameterEvent& event);

    void OnDebounceTimer(wxTimerEvent&);

private:
    wxString m_pluginId;

    wxBoxSizer* m_headerSizer;

    bool m_isActive{ false };

    wxString m_note;

    double m_score{ -1.0 };

    std::optional<double> m_debouncedThr{ -1.0 };

    double m_threshold{ -1.0 };

	bool m_isThresholdEnabled{ true };

	bool m_isDebugResultOn{ false };

    bool m_hasDebugResult{ false };

    std::vector<std::shared_ptr<ParameterInfo>> m_params;

    wxCheckBox* m_isActiveItem;

    wxStaticText* m_noteItem;

    wxStaticText* m_scoreItem;

    wxSpinCtrlDouble* m_thresholdItem;

    wxToggleButton* m_showDebugResult;

    wxBitmapButton* m_pluginParametersBtn;

    EvaluationPluginParametersFrame* m_pluginParametersFrame;

    wxTimer m_debounceTimer;
};
