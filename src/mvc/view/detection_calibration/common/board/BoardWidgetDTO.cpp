#include "BoardWidgetDTO.hpp"


BoardWidgetDTO::BoardWidgetDTO(
    std::uint32_t mapId,
    wxString note,
    BoardStatus boardStatus,
    EvaluationStatus evalCode,
    std::unordered_map<wxString, wxString> metadata)
    : m_mapId(mapId),
    m_note(std::move(note)),
    m_boardStatus(boardStatus),
    m_evalCode(evalCode),
    m_metadata(std::move(metadata))
{
}

/////////////////////////////////////////////////////////

std::uint32_t BoardWidgetDTO::GetMapId() const
{
    return m_mapId;
}

const wxString& BoardWidgetDTO::GetNote() const
{
    return m_note;
}

BoardWidgetDTO::BoardStatus BoardWidgetDTO::GetBoardStatus() const
{
    return m_boardStatus;
}

BoardWidgetDTO::EvaluationStatus BoardWidgetDTO::GetEvaluationCode() const
{
    return m_evalCode;
}

const std::unordered_map<wxString, wxString>& BoardWidgetDTO::GetMetadata() const
{
    return m_metadata;
}