#include "BoardWidgetList.hpp"
#include "BoardWidget.hpp"
#include "BoardWidgetDTO.hpp"
#include "BoardWidgetEvents.hpp"


BoardWidgetList::BoardWidgetList(
	wxWindow* parent,
	wxWindowID winid,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxString& name)
	: 
    BaseWidgetList(parent,
        winid,
        pos,
        size,
        style,
        name)
{
}

//////////////////////////////////////////////////////////////////////////

bool BoardWidgetList::AppendBoard(const BoardWidgetDTO& boardDTO)
{
    auto it = m_boardIdtoItem.find(boardDTO.GetMapId());
    if (it != m_boardIdtoItem.end())
        return false;

    std::shared_ptr<BoardWidget> board = std::make_shared<BoardWidget>(this, boardDTO);

    m_boardIdtoItem[boardDTO.GetMapId()] = board;
    m_itemToBoardId[board] = boardDTO.GetMapId();

    // Bind custom event
    board->Bind(GUI_BOARD_CLICK, &BoardWidgetList::OnWidgetClicked, this);

    BaseWidgetList::AppendWidget(board);

    return true;
}

std::shared_ptr<BoardWidget> BoardWidgetList::GetWidget(std::uint32_t boardId)
{
    auto it = m_boardIdtoItem.find(boardId);
    if (it == m_boardIdtoItem.end())
        return nullptr;

    return it->second;
}

void BoardWidgetList::RemoveBoard(std::uint32_t toRemoveId)
{
    auto it = m_boardIdtoItem.find(toRemoveId);
    if (it == m_boardIdtoItem.end())
        return;


    std::shared_ptr<BoardWidget> board = it->second;

    m_boardIdtoItem.erase(toRemoveId);

    m_itemToBoardId.erase(board);

    BaseWidgetList::RemoveWidget(board);
}

bool BoardWidgetList::SelectBoard(std::uint32_t id)
{
    auto it = m_boardIdtoItem.find(id);
    if (it == m_boardIdtoItem.end()) return false;

    it->second->SetSelected(true);
    m_selectedWidget = it->second;

    return true;
}

std::optional<std::uint32_t> BoardWidgetList::GetSelectedBoardId() const
{
    if (!m_selectedWidget) return std::nullopt;

    std::shared_ptr<BoardWidget> selectedBoard = std::static_pointer_cast<BoardWidget>(m_selectedWidget);

    auto it = m_itemToBoardId.find(selectedBoard);
    if (it == m_itemToBoardId.end()) return std::nullopt;

    return it->second;
}

//////////////////////////////////////////////////////////////////////////

void BoardWidgetList::RemoveSelectedWidgetImpl(const std::shared_ptr<BaseWidget>& baseWidget)
{
    std::shared_ptr<BoardWidget> toRemoveWidget = std::static_pointer_cast<BoardWidget>(baseWidget);

    const std::uint32_t toRemoveId = m_itemToBoardId.at(toRemoveWidget);

    m_itemToBoardId.erase(toRemoveWidget);

    m_boardIdtoItem.erase(toRemoveId);
}

void BoardWidgetList::RemoveAllWidgetsImpl()
{
    m_boardIdtoItem.clear();
    m_itemToBoardId.clear();
}