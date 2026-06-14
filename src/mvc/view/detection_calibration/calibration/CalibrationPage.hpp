#pragma once
#include "../common/CalibrationStageView.hpp"


// Forward declarations
class wxStaticText;
class wxToggleButton;
class ParameterWidgetList;
class BusyOverlay;


/**
 * @brief View responsible for managing the actual calibration step.
 */
class CalibrationPage : public CalibrationStageView
{
public:
    //-----------------------------------------------------------------------------
    // Enums
    //-----------------------------------------------------------------------------

    /**
     * @enum ParameterLocation
     * @brief Specifies which sub-panel container hosts a given set of parameters.
     */
    enum class ParameterLocation {
        SETUP,              ///< General configuration parameter block.
        CAMERA_MATRIX,      ///< Intrinsic camera matrix parameters.
        DISTORTION_MODEL    ///< Lens distortion coefficients.
    };

    /**
     * @enum UiState
     * @brief Layout configuration state representing the hardware/computation execution mode.
     */
    enum class UiState {
        IDLE,               ///< Standard ready state.
        CALIBRATION         ///< Computation active. Disables standard inputs.
    };

    /**
     * @enum Page
     * @brief Indices representing the main functional tabs of the notebook workspace.
     */
    enum class Page {
        SETUP,              ///< Tab index for detection configuration and inputs.
        CALIBRATION         ///< Tab index for executing and analyzing calibration metrics.
    };


    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    CalibrationPage(
        wxWindow* parent,
        wxWindowID winid = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxTAB_TRAVERSAL | wxNO_BORDER,
        const wxString& name = wxASCII_STR(wxPanelNameStr)
    );

    ~CalibrationPage() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /**
     * @brief Switches the currently active view tab index frame layout.
     * @param page The target view page destination enum entry.
     * @return The raw integer index value of the newly selected notebook page.
     */
    int GoToPage(Page page);

    /**
     * @brief Updates the visual text tracking total calibration error metric.
     * @param error Numerical re-projection error value.
     */
    void SetCalibrationError(double error);

    /**
     * @brief Populates the parameter panels within the view.
     * @param params Vector containing configurations blocks.
     * @param where Identifies which target sub-panel layout hosts these parameters.
     */
    void SetParameters(
        const std::vector<std::shared_ptr<ParameterInfo>>& params,
        ParameterLocation where
    );

    /** @brief Updates a specific parameters inside the view tracker. */
    void UpdateParameter(const std::shared_ptr<ParameterInfo>& param) override;

    /** @brief Flags a parameter visually to notify users that it has unsaved edits. */
    void MarkParameterAsDirty(
        const wxString& paramId,
        bool isDirty
    ) override;

    /**
     * @brief Transitions the page into a specific execution layout state.
     *
     * Automatically activates/deactivates button configurations to match state safety requirements.
     */
    void SetUiState(UiState uiState);

    /**
     * @brief Explicitly toggles the state tracking system for active real-time undistortion previewing.
     * @param check True forces active tracking; false disables mathematical matrix correction mapping.
     */
    void CheckUndistort(bool check = true);

    /** @brief Convienence method helper utility wrapper function shortcut disabling active undistortion previewing. */
    void UncheckUndistort() { CheckUndistort(false); }

    /**
     * @brief Queries the visual toggle check interaction element flag state tracker.
     * @return Current status configuration matching interface active flags.
     */
    bool IsUndistortChecked() const;

    /**
     * @brief Explicitly toggles the state tracking system for auto calibrate on new dataset.
     * @param check True forces active tracking; false disables.
     */
    void CheckAutoCalibrateOnNewDataset(bool check = true) ;

    /** 
     * @brief Queries the visual toggle check interaction element flag state tracker
     *  for auto-calibrate on new dataset.
     */
    bool IsAutoCalibrateOnNewDatasetChecked() const;

private:
    wxNotebook* CreateNotebook();

    wxPanel* CreateSetupCalibrationTab(wxWindow* parent);

    wxPanel* CreateCalibrationSubTab(wxWindow* parent);

    wxPanel* CreateImageDisplayPanel(wxWindow* parent);

    wxPanel* CreateCalibSummaryPanel(wxWindow* parent);

    wxPanel* CreateDatasetPanel(wxWindow* parent);

    void DatasetUpdated() override;

    void UpdateCalibrateBtn();

    void IdleMode();

    void CalibrationMode();


    //-----------------------------------------------------------------------------
    // Events
    //-----------------------------------------------------------------------------

    void OnCalibrate(wxCommandEvent& event);

    void OnSaveCalibrationResult(wxCommandEvent& event);

    void OnUndistort(wxCommandEvent& event);

    void OnClose(wxEvent& event);

private:
    UiState m_uiState;

    wxBitmapButton* m_saveCalibResultBtn;

    wxStaticText* m_calibError;

    wxCheckBox* m_autoCalibOnNewDataset;

    wxPanel* m_setupDetectionTab;

    wxPanel* m_calibrationTab;

    ParameterWidgetList* m_calibParamsList;

    ParameterWidgetList* m_camMatrixParamsList;

    ParameterWidgetList* m_distModelParamsList;

	BusyOverlay* m_busyOverlayCamMatrix;

	BusyOverlay* m_busyOverlayDistModel;

	wxButton* m_calibrateBtn;

    wxBitmapButton* m_loadModelParams;

    wxBitmapButton* m_saveModelParams;

    wxToggleButton* m_undistortBtn;
};
