#pragma once
#include <cstdint>
#include <unordered_map>
#include <wx/string.h>


/**
 * @brief Data Transfer Object representing the state of a board widget.
 */
class BoardWidgetDTO
{
public:
    //-----------------------------------------------------------------------------
    // Constants
    //-----------------------------------------------------------------------------

    enum class BoardStatus {
        NONE,
        DETECTED,
        NOT_DETECTED,
        POSE_NOT_SOLVED
    };

    enum class EvaluationStatus {
        NOT_EVALUATED,
        GOOD,
        HAS_ISSUES
    };


    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    /**
     * @brief Constructs a fully initialized, immutable BoardWidgetDTO instance.
     * @param mapId Unique identifier matching this widget element back to the system map.
     * @param note A contextual user or system narrative remark.
     * @param boardStatus The current state of hardware detection or spatial mathematical tracking.
     * @param evalCode The diagnostic quality outcome of the component.
     * @param metadata Extensible key-value pairs of raw string properties.
     */
    BoardWidgetDTO(
        std::uint32_t mapId,
        wxString note,
        BoardStatus boardStatus,
        EvaluationStatus evalCode,
        std::unordered_map<wxString, wxString> metadata = {}
    );

    ~BoardWidgetDTO() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /** @brief Gets the unique layout map identifier. */
    std::uint32_t GetMapId() const;

    /** @brief Gets the custom narrative remark or note text string. */
    const wxString& GetNote() const;

    /** @brief Gets the hardware tracking and localization status flag. */
    BoardStatus GetBoardStatus() const;

    /** @brief Gets the quality validation code of the board process. */
    EvaluationStatus GetEvaluationCode() const;

    /** @brief Gets custom extensible string attributes assigned to this board instance. */
    const std::unordered_map<wxString, wxString>& GetMetadata() const;

private:
    // Unique component identifier.
    std::uint32_t m_mapId;

    // Informational text message.
    wxString m_note;

    // Spatial tracking status tracker.
    BoardStatus m_boardStatus;

    // Diagnostic tracking status tracker.
    EvaluationStatus m_evalCode;

    // Dynamic property mapping attributes.
    std::unordered_map<wxString, wxString> m_metadata;
};
