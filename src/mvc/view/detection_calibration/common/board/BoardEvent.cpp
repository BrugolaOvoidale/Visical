#include "BoardEvent.hpp"


BoardEvent::BoardEvent(
    wxEventType type,
    int winid)
    : wxCommandEvent(type, winid)
{
}

BoardEvent::BoardEvent(const BoardEvent& e)
    : wxCommandEvent(e),
    m_value(e.m_value)
{
}

wxEvent* BoardEvent::Clone() const
{
    return new BoardEvent(*this);
}

//////////////////////////////////

void BoardEvent::SetBoardId(std::uint32_t value)
{
    m_value = value;
}

std::uint32_t BoardEvent::GetBoardId() const
{
    return m_value;
}