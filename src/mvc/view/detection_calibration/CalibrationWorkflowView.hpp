#pragma once
#include <wx/frame.h>
#include "../BaseView.hpp"


// Forward declarations
class SetupPage;
class DetectionPage;
class CalibrationPage;
class wxNotebook;
class wxBookCtrlEvent;


/**
 * @brief Top-level frame managing the step-by-step calibration user interface.
 *
 * This class orchestrate a tabbed multi-page workflow interface and governs
 * navigation and acts as the parent container for individual steps:
 * Setup, Detection, and Calibration.
 */
class CalibrationWorkflowView : public BaseView, public wxFrame
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    /**
     * @brief Constructs the workflow frame and initializes UI components.
     * @param title The text displayed in the window title bar.
     * @param startShowed If true, immediately calls Show() to render the window.
     */
    CalibrationWorkflowView(
        const wxString& title,
        bool startShowed = true
    );

    ~CalibrationWorkflowView() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /** @brief Returns a pointer to the Setup workflow step panel. */
    SetupPage* GetSetupPage() const;

    /** @brief Returns a pointer to the Detection workflow step panel. */
    DetectionPage* GetDetectionPage() const;

    /** @brief Returns a pointer to the Calibration workflow step panel. */
    CalibrationPage* GetCalibrationPage() const;

    /**
     * @brief Programmatically changes the visible tab to the specified page panel.
     * @param page Pointer to the child window step panel.
     * @return The zero-based index of the newly active page, or 'wxNOT_FOUND' if navigation failed.
     */
    int GoToPage(wxWindow* page);

private:
    //-----------------------------------------------------------------------------
    // Events
    //-----------------------------------------------------------------------------

    void OnPageChanged(wxBookCtrlEvent& event);

    void OnShowDebugFrame(wxCommandEvent& event);

    void OnClose(wxCloseEvent& event);

private:
    wxNotebook* m_notebook;

    int m_currPage{ wxNOT_FOUND };

	SetupPage* m_setupPage;

	DetectionPage* m_detectionPage;

	CalibrationPage* m_calibrationPage;
};
