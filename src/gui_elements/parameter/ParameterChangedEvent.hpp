#pragma once
#include <variant>
#include "ParameterEvent.hpp"


class ParameterChangedEvent : public ParameterEvent
{
public:
    //-----------------------------------------------------------------------------
    // Aliases
    //-----------------------------------------------------------------------------

    using Value = std::variant<std::monostate, wxString, int, double, bool>;


    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    ParameterChangedEvent(wxEventType type = wxEVT_NULL, int winid = 0)
        : ParameterEvent(type, winid)
    {}

    ParameterChangedEvent(const ParameterChangedEvent& e)
        : ParameterEvent(e),
        m_value(e.GetValue())
    {}

    wxEvent* Clone() const override { return new ParameterChangedEvent(*this); }

    ~ParameterChangedEvent() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    void SetValue(Value value) { m_value = value; }

    Value GetValue() const { return m_value; }

private:
    Value m_value;
};
