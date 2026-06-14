#pragma once
#include <wx/event.h>


class PixelEvent : public wxCommandEvent
{
public:
    PixelEvent(wxEventType type = wxEVT_NULL, int winid = 0)
        : wxCommandEvent(type, winid)
    {}

    PixelEvent(const PixelEvent& e)
        : wxCommandEvent(e),
        m_pixel(e.m_pixel)
    {}

    wxEvent* Clone() const override { return new PixelEvent(*this); }

public:
    void SetPixel(const wxPoint& pixel) { m_pixel = pixel; }
    void SetPixel(int x, int y) { m_pixel = {x, y}; }
    const wxPoint& GetPixel() const { return m_pixel; }

private:
    wxPoint m_pixel;
};

// Payload: wxPoint
wxDECLARE_EVENT(GUI_CTRL_HOVER_PIXEL, PixelEvent);
