#pragma once
#include <gui_elements/base_widget/BaseWidgetList.hpp>


// Forward declarations
class BoardWidgetDTO;
class BoardWidget;


/**
 * @brief A scrollable UI container that manages and displays a virtual list of unique Boards.
 */
class BoardWidgetList : public BaseWidgetList
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    BoardWidgetList(
        wxWindow* parent,
        wxWindowID winid = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxVSCROLL | wxSIMPLE_BORDER,
        const wxString& name = wxASCII_STR(wxPanelNameStr)
    );

    ~BoardWidgetList() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /**
     * @brief Instantiates a new Board and appends it to the end of the list view.
     * @return false if board is already in the list, false otherwise.
     */
    bool AppendBoard(const BoardWidgetDTO& boardDTO);

    /**
    * @brief Retrieves a board widget instance by its unique identifier string.
    * @param boardId The unique identifier of the target board widget.
    * @return A shared pointer to the requested BoardWidget, or nullptr if not found.
    */
    std::shared_ptr<BoardWidget> GetWidget(std::uint32_t boardId);

    /**
     * @brief Removes a board widget from the UI list and destroys its tracking references.
     * @param toRemoveId The unique identifier of the board to remove.
     */
    void RemoveBoard(std::uint32_t toRemoveId);

    /**
     * @brief Programmatically changes the selection focus to a specific board widget.
     * @param boardId The string identifier of the board to select.
     * @return true if the board was found and successfully selected, false otherwise.
     */
    bool SelectBoard(std::uint32_t boardId);

    /**
     * @brief Returns the selected board id, if any.
     * @return board id if there is a selected board, std::nullopt otherwise.
     */
    std::optional<std::uint32_t> GetSelectedBoardId() const;

private:
    void RemoveSelectedWidgetImpl(const std::shared_ptr<BaseWidget>& baseWidget) override;

    void RemoveAllWidgetsImpl() override;

private:
    std::unordered_map<std::uint32_t, std::shared_ptr<BoardWidget>> m_boardIdtoItem;

    std::unordered_map<std::shared_ptr<BoardWidget>, std::uint32_t> m_itemToBoardId;
};