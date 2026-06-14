#pragma once
#include <cstdint>
#include <wx/event.h>


/**
 * @brief Custom wxWidgets command event used to propagate board-related notifications.
 */
class BoardEvent : public wxCommandEvent
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    /**
     * @brief Constructs a new Board Event object.
     * @param type The specific type of the event.
     * @param winid The window identifier of the source object emitting this event.
     */
    BoardEvent(wxEventType type = wxEVT_NULL, int winid = 0);

    /**
     * @brief Copy constructor.
     * @param e The source event instance to copy data from.
     */
    BoardEvent(const BoardEvent& e);

    /**
     * @brief Creates a deep copy of the event instance on the heap.
     * @return A pointer to the newly allocated BoardEvent clone.
     */
    wxEvent* Clone() const override;

    ~BoardEvent() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------
    
    /**
     * @brief Sets the unique identifier of the board associated with this event.
     * @param value The 32-bit board tracking Id.
     */
    void SetBoardId(std::uint32_t value);

    /**
     * @brief Retrieves the unique identifier of the board associated with this event.
     * @return The 32-bit board tracking Id.
     */
    std::uint32_t GetBoardId() const;

private:
    std::uint32_t m_value{ 0 };
};
