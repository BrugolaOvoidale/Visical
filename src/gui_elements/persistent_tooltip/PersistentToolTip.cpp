#include "PersistentToolTip.hpp"
#include <wx/log.h>
#include <wx/timer.h>
#include <wx/panel.h>
#include <wx/stattext.h>
#include <wx/sizer.h>


PersistentToolTip::PersistentToolTip(
    wxWindow* target,
    const wxString& text,
    bool isManual)
    : wxFrame(target, wxID_ANY, wxEmptyString,
    wxDefaultPosition, wxDefaultSize,
    wxFRAME_NO_TASKBAR | wxSTAY_ON_TOP | wxFRAME_TOOL_WINDOW | wxSIMPLE_BORDER),
    m_target(target),
    m_delayMs(500),
    m_autoPopMs(0),
    m_reshowMs(100),
    m_maxWidth(600),
    m_enabled(true),
    m_wasVisible(false),
    m_isManual(false),
    m_delayTimer(this, ID_DELAY_TIMER),
    m_autoPopTimer(this, ID_AUTOPOP_TIMER)
{
    BuildUI();

    SetText(text);

    Bind(wxEVT_TIMER, &PersistentToolTip::OnDelayTimer, this, ID_DELAY_TIMER);
    Bind(wxEVT_TIMER, &PersistentToolTip::OnAutoPopTimer, this, ID_AUTOPOP_TIMER);
}

PersistentToolTip::~PersistentToolTip()
{
    m_delayTimer.Stop();
    m_autoPopTimer.Stop();

    if (s_globalToolTip == this)
		s_globalToolTip = nullptr;

    if (m_isInitialized)
    {
        m_target->Unbind(wxEVT_ENTER_WINDOW, &PersistentToolTip::OnTargetEnter, this);
        m_target->Unbind(wxEVT_LEAVE_WINDOW, &PersistentToolTip::OnTargetLeave, this);
    }
}

///////////////////////////////////////////////////////////////////////////////////////

void PersistentToolTip::SetToolTip(
    wxWindow* target,
    const wxString& text)
{
    if (text.IsEmpty())
        return;

    s_instances[target] = text;

    target->Bind(wxEVT_ENTER_WINDOW, &PersistentToolTip::sOnTargetEnter);
}

void PersistentToolTip::RemoveToolTip(wxWindow* target)
{
    auto it = s_instances.find(target);
    if (it == s_instances.end())
        return;

    target->Unbind(wxEVT_ENTER_WINDOW, &PersistentToolTip::sOnTargetEnter);

    s_instances.erase(target);
}

void PersistentToolTip::SetDelay(int ms)
{
    m_delayMs = ms;
}

void PersistentToolTip::SetAutoPop(int ms)
{
    m_autoPopMs = ms;
}

void PersistentToolTip::SetReshow(int ms)
{ 
    m_reshowMs = ms;
}

void PersistentToolTip::SetMaxWidth(int width)
{
    m_maxWidth = width;
    ApplyMaxWidth();
}

void PersistentToolTip::SetManual(bool isManual)
{
    if (m_isManual == isManual)
        return;

    m_isManual = isManual;

    if (m_isManual)
    {
        // Detach auto-show machinery if it was already wired up
        if (m_isInitialized)
        {
            m_target->Unbind(wxEVT_ENTER_WINDOW, &PersistentToolTip::OnTargetEnter, this);
            m_target->Unbind(wxEVT_LEAVE_WINDOW, &PersistentToolTip::OnTargetLeave, this);
            m_isInitialized = false;
        }
        DismissNow();
    }
    else
    {
        // Re-arm auto-show if there is text to show
        if (!m_text.IsEmpty() && !m_isInitialized)
        {
            m_target->Bind(wxEVT_ENTER_WINDOW, &PersistentToolTip::OnTargetEnter, this);
            m_target->Bind(wxEVT_LEAVE_WINDOW, &PersistentToolTip::OnTargetLeave, this);
            m_isInitialized = true;
        }
    }
}

int PersistentToolTip::GetMaxWidth() const
{
    return m_maxWidth >= 0 ? m_maxWidth : 0;
}

void PersistentToolTip::SetText(const wxString& text)
{
    if (m_text == text)
        return;

    m_text = text;

    if (text.IsEmpty())
    {
        DismissNow();

        if (m_isInitialized)
        {
            m_target->Unbind(wxEVT_ENTER_WINDOW, &PersistentToolTip::OnTargetEnter, this);
            m_target->Unbind(wxEVT_LEAVE_WINDOW, &PersistentToolTip::OnTargetLeave, this);

            m_isInitialized = false;
        }

        return;
    }

    if (!m_isInitialized && m_target)
    {
        m_target->Bind(wxEVT_ENTER_WINDOW, &PersistentToolTip::OnTargetEnter, this);
        m_target->Bind(wxEVT_LEAVE_WINDOW, &PersistentToolTip::OnTargetLeave, this);

        m_isInitialized = true;
    }

    ApplyMaxWidth();
}

void PersistentToolTip::ShowAt(const wxPoint& screenPos, const wxString& text)
{
    if (!IsEnabled()) return;

    SetText(text);

    Move(screenPos.x + 12, screenPos.y + 12);

    Show();
}

///////////////////////////////////////////////////////////////////////////////////////

void PersistentToolTip::BuildUI()
{
    m_panel = new wxPanel(this);
    m_panel->SetBackgroundColour(wxColour(255, 255, 255));

    m_label = new wxStaticText(m_panel, wxID_ANY, m_text);
    ApplyMaxWidth();

    wxBoxSizer* inner = new wxBoxSizer(wxVERTICAL);
    inner->Add(m_label, 0, wxALL, 4);
    m_panel->SetSizerAndFit(inner);

    wxBoxSizer* outer = new wxBoxSizer(wxVERTICAL);
    outer->Add(m_panel, 1, wxEXPAND);
    SetSizerAndFit(outer);
}

bool PersistentToolTip::IsEnabled() const
{
    return m_enabled;
}

int PersistentToolTip::GetDelay() const
{
    return m_delayMs;
}

int PersistentToolTip::GetAutoPop() const
{
    return m_autoPopMs;
}

int PersistentToolTip::GetReshow() const
{
    return m_reshowMs;
}

void PersistentToolTip::ApplyMaxWidth()
{
    int maxWidth = GetMaxWidth();

    m_label->SetLabel(m_text);    // reset before re-wrapping

    if (maxWidth > 0)
        m_label->Wrap(maxWidth);

    m_label->InvalidateBestSize();
    m_panel->SetSizerAndFit(m_panel->GetSizer()); // refit the inner panel
    SetSizerAndFit(GetSizer());                   // refit the popup window itself
    Layout();
}

void PersistentToolTip::ShowTooltip(const wxPoint& screenPos)
{
    wxPoint clientPos = m_target->ClientToScreen(screenPos);
    Move(clientPos.x + 12, clientPos.y + 12);

    Show();

    m_wasVisible = true;

    int autoPop = GetAutoPop();
    if (autoPop > 0)
        m_autoPopTimer.StartOnce(autoPop);
}

void PersistentToolTip::ShowNow(const wxPoint& screenPos)
{
    m_autoPopTimer.Stop();

    int delay = m_wasVisible ? GetReshow() : GetDelay();

    if (delay <= 0) ShowTooltip(screenPos);
    else
    {
        m_delayedPos = screenPos;

        m_delayTimer.StartOnce(delay);
    }
}

void PersistentToolTip::DismissNow()
{
    m_delayTimer.Stop();
    m_autoPopTimer.Stop();

    Hide();
}

void PersistentToolTip::sOnTargetEnter(wxMouseEvent& e)
{
    wxWindow* target = wxStaticCast(e.GetEventObject(), wxWindow);
    auto it = s_instances.find(target);
    if (it == s_instances.end())
        return;

    if (!s_globalToolTip)
    {
        s_globalToolTip = new PersistentToolTip(target, it->second);
    }
    else
    {
        if (s_globalToolTip->GetParent() != target)
        {
            s_globalToolTip->Destroy();

            s_globalToolTip = nullptr;

            s_globalToolTip = new PersistentToolTip(target, it->second);

            s_globalToolTip->ShowNow(e.GetPosition());
        }
    }

    e.Skip();
}

void PersistentToolTip::OnTargetEnter(wxMouseEvent& e)
{
    if (IsEnabled())
        ShowNow(e.GetPosition());

    e.Skip();
}

void PersistentToolTip::OnTargetLeave(wxMouseEvent& e)
{
    DismissNow();
    e.Skip();
}

void PersistentToolTip::OnDelayTimer(wxTimerEvent&)
{
    ShowTooltip(m_delayedPos);
}

void PersistentToolTip::OnAutoPopTimer(wxTimerEvent&)
{
    Hide();

    m_wasVisible = false;
}