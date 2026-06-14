#include "BusyOverlay.hpp"
#include <wx/toplevel.h>
#include <wx/sizer.h>
#include <wx/activityindicator.h>
#include <wx/stattext.h>
#include <wx/dcbuffer.h>


BusyOverlay::BusyOverlay(wxWindow* parent)
    : wxWindow(parent, wxID_ANY,
        wxDefaultPosition, wxDefaultSize,
        wxTRANSPARENT_WINDOW | wxWANTS_CHARS | wxBORDER_NONE),
    m_parent(parent)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    // SetTransparent(150); // semi-transparent

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    // Spinner
    m_spinner = new wxActivityIndicator(this);
    m_spinner->SetBackgroundColour(wxColour(128, 128, 128, 100));

    // Override text instead of spinner
    m_text = new wxStaticText(this, wxID_ANY, DEFAULT_LOADING);
    m_text->SetBackgroundColour(wxColour(10, 10, 10, 200));
    m_text->SetForegroundColour(*wxWHITE);

    wxFont font = m_text->GetFont();
    font.SetWeight(wxFONTWEIGHT_BOLD);
    m_text->SetFont(font);


    sizer->AddStretchSpacer();
    sizer->Add(m_text, 0, wxALIGN_CENTER);
    sizer->Add(m_spinner, 0, wxALIGN_CENTER);
    sizer->AddStretchSpacer();
    SetSizer(sizer);

    Hide();

    // Parent events
    m_parent->Bind(wxEVT_SIZE, &BusyOverlay::OnParentUpdate, this);
    m_parent->Bind(wxEVT_SHOW, &BusyOverlay::OnParentShow, this);
    m_parent->Bind(wxEVT_MOVE,    &BusyOverlay::OnParentUpdate, this);

    // Top-level parent move
    m_topParent = wxStaticCast(wxGetTopLevelParent(parent), wxTopLevelWindowBase);
    if (m_topParent && m_topParent != parent)
    {
        m_topParent->Bind(wxEVT_MOVE, &BusyOverlay::OnParentUpdate, this);
        m_topParent->Bind(wxEVT_SIZE, &BusyOverlay::OnParentUpdate, this);
        m_topParent->Bind(wxEVT_ICONIZE, &BusyOverlay::OnParentIconize, this);
    }

    // Paint
    Bind(wxEVT_PAINT, &BusyOverlay::OnPaint, this);
    //Bind(wxEVT_ERASE_BACKGROUND, [](wxEraseEvent&) {}); // prevent flicker
}

//////////////////////////////////////////

void BusyOverlay::ShowOverlay(bool show)
{
    if (show && m_topParent->IsShown() && !m_topParent->IsIconized() && m_parent->IsShown())
    {
        m_showOverlay = true;

        UpdatePosition();

        if (m_useSpinner)
        {
            m_spinner->Show();
            m_spinner->Start();
        
            m_text->Hide();
        }
        else
        {
            m_spinner->Hide();
            m_spinner->Stop();

            if (!m_text->GetLabel().IsEmpty())
                m_text->Show();
        }

        Show();

        Raise();
    }
    else
    {
        m_showOverlay = false;

        m_spinner->Hide();
        m_spinner->Stop();

        m_text->Hide();

        Hide();
    }

    Layout();
    Refresh();
    Update();
}

void BusyOverlay::UseStaticText(const wxString& txt)
{
    m_useSpinner = false;

    if (txt != m_text->GetLabel())
        m_text->SetLabel(txt);
}

void BusyOverlay::UseSpinner()
{
    m_useSpinner = true;
}

//////////////////////////////////////////

void BusyOverlay::UpdatePosition()
{
    if (!m_parent || !m_parent->IsShown())
        return;

    // Use parent's client size and position (0,0) in parent coords
    wxSize size = m_parent->GetClientSize();
    SetSize(0, 0, size.GetWidth(), size.GetHeight());

    // Ensure we stay on top of siblings
    if (IsShown())
        Raise();

    Layout();
    Refresh();
}

void BusyOverlay::OnParentUpdate(wxEvent& evt)
{
    //if (IsShown())
        UpdatePosition();
    evt.Skip();
}

void BusyOverlay::OnParentShow(wxShowEvent& evt)
{
    const bool shouldShow = m_showOverlay && m_parent->IsShown();

    Show(shouldShow);

    evt.Skip();
}

void BusyOverlay::OnParentIconize(wxIconizeEvent& evt)
{
    // Hide overlay when minimized, restore when un-minimized
    if (evt.IsIconized())
        Hide();
    else if (m_showOverlay)
        ShowOverlay(true);
    
    evt.Skip();
}

void BusyOverlay::OnPaint(wxPaintEvent&)
{
    wxAutoBufferedPaintDC dc(this);

    wxBrush brush(wxColour(128, 128, 128, 100)); // semi-transparent gray
    dc.SetBrush(brush);
    dc.SetPen(*wxTRANSPARENT_PEN);

    dc.DrawRectangle(GetClientRect());
}
