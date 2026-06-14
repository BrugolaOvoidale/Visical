#include "CalibrationWorkflowView.hpp"
#include <wx/display.h>
#include <wx/notebook.h>
#include <wx/menu.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include "setup/SetupPage.hpp"
#include "detection/DetectionPage.hpp"
#include "calibration/CalibrationPage.hpp"
#include "CalibrationWorkflowViewEvents.hpp"


static const wxString SELECTED_PAGE{ wxT("• ") };

///////////////////////////////////////////////////

CalibrationWorkflowView::CalibrationWorkflowView(
    const wxString& title,
    bool startShowed)
    : BaseView(this),
    wxFrame(nullptr, wxID_ANY, title)
{
    // Create the File menu
    wxMenu* fileMenu = new wxMenu;
    fileMenu->AppendSeparator();
    fileMenu->Append(wxID_EXIT, "E&xit\tAlt+F4", "Quit this program");

    // Create the Help menu
    wxMenu* helpMenu = new wxMenu;
    helpMenu->Append(wxID_ABOUT, "&About\tF1", "Show information about this program");

    // Create the menu bar and attach the menus
    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(fileMenu, "&File");
    menuBar->Append(helpMenu, "&Help");

    menuBar->SetBackgroundColour(GetBackgroundColour());

    // Attach the menu bar to the frame
    SetMenuBar(menuBar);

    wxPanel* mainPanel = new wxPanel(this);

    wxIcon icon;
    icon.LoadFile("resources/appIcon.ico", wxBITMAP_TYPE_ICO);
    SetIcon(icon);


    // Create the notebook and add tabs
    m_notebook = new wxNotebook(mainPanel, wxID_ANY);

    m_setupPage = new SetupPage(m_notebook);

    m_detectionPage = new DetectionPage(m_notebook);

    m_calibrationPage = new CalibrationPage(m_notebook);

    m_notebook->AddPage(m_setupPage, SELECTED_PAGE + "Setup", true);
    m_notebook->AddPage(m_detectionPage, "Detection");
    m_notebook->AddPage(m_calibrationPage, "Calibration");

    m_currPage = 0;

    m_notebook->Bind(wxEVT_NOTEBOOK_PAGE_CHANGED, &CalibrationWorkflowView::OnPageChanged, this);

    wxButton* showDebugFrame = new wxButton(mainPanel, wxID_ANY, "Show debug logs");
    showDebugFrame->Bind(wxEVT_BUTTON, &CalibrationWorkflowView::OnShowDebugFrame, this);

    // Create a vertical sizer and add the button above the notebook.
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->Add(m_notebook, 1, wxEXPAND | wxALL, 0);
    mainSizer->Add(showDebugFrame, 0, wxALIGN_RIGHT | wxALL, 0);
    mainPanel->SetSizerAndFit(mainSizer);
    mainPanel->SetMinSize(mainPanel->GetSize());

    // DPI-aware minimum for the whole frame
    const wxSize kMinFrameSize = FromDIP(mainSizer->GetMinSize());
    SetMinClientSize(kMinFrameSize);

    wxDisplay display(wxDisplay::GetFromWindow(this));
    wxRect screenRect = display.GetClientArea();

    // Initial size: 80%x80% of screen, but never smaller than our minimum
    const int width = std::max((int)(screenRect.GetWidth() * 0.8), kMinFrameSize.x);
    const int height = std::max((int)(screenRect.GetHeight() * 0.8), kMinFrameSize.y);
    SetSize(wxSize(width, height));

    Centre();

    Bind(wxEVT_CLOSE_WINDOW, &CalibrationWorkflowView::OnClose, this);
    Bind(wxEVT_CLOSE_WINDOW, &CalibrationWorkflowView::OnClose, this, wxID_EXIT);

    Maximize(true);
    Layout();
    
    Show(startShowed);
}

///////////////////////////////////////////////////

SetupPage* CalibrationWorkflowView::GetSetupPage() const
{
    return m_setupPage;
}

DetectionPage* CalibrationWorkflowView::GetDetectionPage() const
{
    return m_detectionPage;
}

CalibrationPage* CalibrationWorkflowView::GetCalibrationPage() const
{
    return m_calibrationPage;
}

int CalibrationWorkflowView::GoToPage(wxWindow* page)
{
     return m_notebook->SetSelection(m_notebook->FindPage(page));
}

///////////////////////////////////////////////////////////////////////

void CalibrationWorkflowView::OnPageChanged(wxBookCtrlEvent& event)
{
    {
        wxString newTxt = m_notebook->GetPageText(m_currPage);
        newTxt.Replace(SELECTED_PAGE, wxEmptyString, false);

        m_notebook->SetPageText(m_currPage, newTxt);
    }

    {
        int sel = event.GetSelection();

        m_currPage = sel;

        wxString newTxt = m_notebook->GetPageText(m_currPage);
        newTxt.Prepend(SELECTED_PAGE);

        m_notebook->SetPageText(m_currPage, newTxt);
    }
}

void CalibrationWorkflowView::OnShowDebugFrame(wxCommandEvent& event)
{
    // Let the parent know this board was clicked
    wxCommandEvent evt(GUI_DEBUG_FRAME, GetId());

    // Send it to parent
    ProcessEvent(evt);         // sends to this and upwards
}

void CalibrationWorkflowView::OnClose(wxCloseEvent& event)
{
    if (event.CanVeto())
    {
        event.Veto();

        // Let the parent know to close the app
        wxCommandEvent evt(GUI_CLOSE_APP, GetId());

        // Send it to parent
        ProcessEvent(evt);         // sends to this and upwards

        return;
    }

    event.Skip(); // allow normal destruction
}