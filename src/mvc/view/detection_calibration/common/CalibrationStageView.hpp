#pragma once
#include <unordered_map>
#include <wx/panel.h>
#include "../../BaseView.hpp"
#include "evaluation/EvaluationPanel.hpp"
#include "board/BoardWidgetDTO.hpp"


// Forward declarations
class wxNotebook;
class wxBookCtrlEvent;
class wxCheckBox;
class ImagePanel;
class BoardWidgetList;
class PluginContextInfo;
class ParameterInfo;
class PluginResult;


/**
 * @brief Abstract base view class managing the calibration UI pipeline stage.
 * 
 * This class orchestrates the presentation layer for image visualization, calibration
 * board inventory tracking, and dynamic image processing/evaluation plugins.
 */
class CalibrationStageView : public BaseView, public wxPanel
{
public:
    //-----------------------------------------------------------------------------
    // Structs
    //-----------------------------------------------------------------------------

    /**
     * @struct DatasetStatus
     * @brief Metrics representing the current processing and evaluation state of the active dataset.
     */
    struct DatasetStatus
    {
        size_t total{ 0 };        ///< Total number of items/frames in the dataset.
        size_t detected{ 0 };     ///< Number of items where calibration targets were detected.
        size_t good{ 0 };         ///< Number of items meeting quality thresholds.
        size_t haveIssues{ 0 };   ///< Number of items flagged with processing errors or low quality.

        /** @brief Resets all counters to zero. */
        void Clear()
        {
            total = 0;
            detected = 0;
            good = 0;
            haveIssues = 0;
        }

        /** @brief Compares equality based on exact metric matches. */
        bool operator==(const DatasetStatus& other)
        {
            return total == other.total &&
                detected == other.detected &&
                good == other.good &&
                haveIssues == other.haveIssues;
        }
    };


    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    ~CalibrationStageView() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /** @brief Replaces the currently displayed frame with a new image. */
    void UpdateImageDisplay(const wxImage& image);

    /** @brief Sets a floating tooltip message tied to specific pixel coordinates. */
    void SetPixelToolTip(const wxString& tip);

    /** @brief Clears the floating pixel tooltip. */
    void UnsetPixelToolTip();

    /** @brief Clears out the image viewport completely. */
    void ClearImageDisplay();

    /** @brief Toggles rendering overlays of the physical calibration board structure. */
    void DrawBoard(bool draw);

    /** @brief Checks if the "Draw Board" visualization option is currently enabled. */
    bool IsDrawBoardEnabled() const;

    /** @brief Toggles rendering overlays of identified interest marks/features. */
    void DrawMarks(bool draw);

    /** @brief Checks if the "Draw Marks" visualization option is currently enabled. */
    bool IsDrawMarksEnabled() const;

    /** @brief Toggles rendering overlays of the World Coordinate System (WCS) axes. */
    void DrawWCS(bool draw);

    /** @brief Checks if the "Draw WCS" visualization option is currently enabled. */
    bool IsDrawWCSEnabled() const;

    /** @brief Deselects any currently highlighted board in the list. */
    void UnselectBoard();

    /** @brief Checks if a board is actively selected by the user. */
    bool IsBoardSelected() const;

    /** @brief Returns the ID of the selected board, or 'std::nullopt' if none is highlighted. */
    std::optional<std::uint32_t> GetSelectedBoard() const;

    /**
     * @brief Updates UI presentation data for a specific board.
     * @param boardDTO The transfer object containing updated property values.
     * @param needAttention Highlights the entry visually if set to true.
     * @return true if the board was found and updated successfully.
     */
    bool UpdateBoard(
        const BoardWidgetDTO& boardDTO,
        bool needAttention = true
    );

    /**
     * @brief Appends a new board tracking element to the view collection.
     * @param boardDTO Initialization properties for the board layout.
     * @param needAttention Sets a warning highlight state initially.
     */
    void AppendBoard(
        const BoardWidgetDTO& boardDTO,
        bool needAttention = true
    );

    /** @brief Forces selection highlight onto a specific board item. */
    virtual void SelectBoard(std::uint32_t id);

    /** @brief Removes a unique board entry from the visualization tracking data. */
    void RemoveBoard(std::uint32_t id);

    /** @brief Wipes out all tracked calibration board entries from the list layout. */
    void RemoveAllBoards();

     /** @brief Checks whether the debug view plugin layer is actively toggled. */
    bool IsShowDebPluginSelected() const;

    /**
     * @brief Configures which plugin is displayed inside the evaluation layout zone.
     * @param pluginId Unique identifier string of the destination plugin.
     * @param where Destination sub-pane location context.
     * @return true if assignment succeeded.
     */
    bool SetSelectedDebPlugin(
        const wxString& pluginId,
        EvaluationPanel::PluginLocation where
    );

    /**
     * @brief Flags a plugin configuration attribute as modified/dirty.
     * @param pluginId Targeted plugin identifier.
     * @param paramId Unique property keyword.
     * @param categoryId Unique category keyword.
     * @param isDirty true to mark it out-of-sync; false to mark clean.
     */
    void MarkPluginParameterAsDirty(
        const wxString& pluginId,
        const wxString& paramId,
        const wxString& categoryId,
        bool isDirty
    );

    /** @brief Returns the Id and active layout pane context of the selected debug plugin. */
    std::optional<std::pair<wxString, EvaluationPanel::PluginLocation>> GetSelectedDebPlugin() const;

    /** @brief Enables execution tracking state for a specific evaluation plugin. */
    void EnablePlugin(
        const wxString& pluginId,
        bool enable = true
    );

    /** @brief Convienence inline alternative disabling execution on a given plugin identifier. */
    void DisablePlugin(const wxString& pluginId) { EnablePlugin(pluginId, false); }

    /** @brief Synchronizes property metadata values inside the designated plugin contextual view. */
    void UpdatePlugin(const std::shared_ptr<PluginContextInfo>& pluginCtx);

    /** @brief Pushes dynamic math execution outputs or logs into the UI display context. */
    void UpdatePluginResult(const std::shared_ptr<PluginResult>& pluginResult);

    /** @brief Mass-applies a uniform threshold across an entire sub-pane layout block. */
    void UpdateAllPluginsThreshold(
        double threshold,
        EvaluationPanel::PluginLocation where
    );

    /** @brief Appends an individual user plugin workspace configuration to a layout area. */
    void AppendPlugin(
        const std::shared_ptr<PluginContextInfo>& pluginCtx,
        EvaluationPanel::PluginLocation where
    );

    /** @brief Requests that all plugins preview can be added to the list. */
    void EnableAllPluginsPreview(EvaluationPanel::PluginLocation where);

    /** @brief Requests that a specific plugin preview can be added to the list. */
    void EnablePluginPreview(const wxString& pluginId);

    /** @brief Requests that a specific plugin preview cannot be added to the list. */
    void DisablePluginPreview(
        const wxString& pluginId,
        EvaluationPanel::PluginLocation where
    );

    /** @brief Set which plugin can be added to the list. */
    void SetPluginsPreview(
        const std::vector<std::shared_ptr<PluginContextInfo>>& plugins,
        EvaluationPanel::PluginLocation where
    );

    /** @brief Resets plugin visual output. */
    void ClearPlugin(const wxString& toClearId);

    /** @brief Resets all plugins visual output inside a layout area. */
    void ClearAllPlugins(EvaluationPanel::PluginLocation where);

    /** @brief Remove a plugin from a specific list. */
    void RemovePlugin(const wxString& toRemoveId);

    /** @brief Remove all plugins from a specific list. */
    void RemoveAllPlugins(EvaluationPanel::PluginLocation where);

    /**
     * @brief Visually updates a parameter.
     * @param param Shared pointer of a ParameterInfo.
     */
    virtual void UpdateParameter(const std::shared_ptr<ParameterInfo>& param) = 0;

    /**
     * @brief Visually updates a parameter dirtiness.
     * @param paramId Unique parameter Id.
     * @param isDirty State assertion.
     */
    virtual void MarkParameterAsDirty(
        const wxString& paramId,
        const wxString& categoryId,
        bool isDirty
    ) = 0;

protected:
    CalibrationStageView(
        wxWindow* parent,
        wxWindowID winid = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxTAB_TRAVERSAL | wxNO_BORDER,
        const wxString& name = wxASCII_STR(wxPanelNameStr)
    );

    virtual void DatasetUpdated() = 0;

    void RemoveBoardContribution(const BoardWidgetDTO& dto);

    void AddBoardContribution(const BoardWidgetDTO& dto);


    //-----------------------------------------------------------------------------
    // Events
    //-----------------------------------------------------------------------------

    void OnPageChanged(wxBookCtrlEvent& event);

    void OnLoadSettings(wxCommandEvent& event);

    void OnSaveSettings(wxCommandEvent& event);

    void OnLoadModelConfig(wxCommandEvent& event);

    void OnSaveModelConfig(wxCommandEvent& event);

    void OnDrawBoard(wxCommandEvent& event);

    void OnDrawMarks(wxCommandEvent& event);
    
    void OnDrawWCS(wxCommandEvent& event);

    void OnClose(wxEvent& event);

protected:
    DatasetStatus m_datasetStatus;

    std::unordered_map<std::uint32_t, BoardWidgetDTO> m_renderedDataset;

    wxNotebook* m_notebook;

    int m_currPage{ wxNOT_FOUND };

    wxBitmapButton* m_loadConfig;

    wxBitmapButton* m_saveConfig;

    wxCheckBox* m_drawBoard;

    wxCheckBox* m_drawMarks;

    wxCheckBox* m_drawWCS;

    ImagePanel* m_imagePanel;

    BoardWidgetList* m_boardsList;

	EvaluationPanel* m_evaluationPanel;

    std::uint32_t m_attentionBoard{ 0 };
};
