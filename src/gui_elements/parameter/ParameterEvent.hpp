#pragma once
#include <wx/event.h>


class ParameterEvent : public wxCommandEvent
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    ParameterEvent(wxEventType type = wxEVT_NULL, int winid = 0)
        : wxCommandEvent(type, winid)
    {}

    ParameterEvent(const ParameterEvent& e)
        : wxCommandEvent(e),
        m_paramId(e.GetParameterId()),
        m_categoryId(e.GetCategoryId())
    {}

    wxEvent* Clone() const override { return new ParameterEvent(*this); }

    virtual ~ParameterEvent() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    void SetParameterId(const wxString& paramId) { m_paramId = paramId; }

    wxString GetParameterId() const { return m_paramId; }

    void SetCategoryId(const wxString& paramId) { m_categoryId = paramId; }

    wxString GetCategoryId() const { return m_categoryId; }

private:
    wxString m_paramId;

    wxString m_categoryId;
};
