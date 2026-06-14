#include "BaseWidgetList.hpp"
#include "BaseWidget.hpp"


BaseWidgetList::BaseWidgetList(
	wxWindow* parent,
	wxWindowID winid,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxString& name)
	: wxScrolledWindow(parent,
		winid,
		pos,
		size,
        style,
		name)
{
    SetScrollRate(0, 10);

    Bind(wxEVT_SCROLLWIN_TOP, &BaseWidgetList::OnScroll, this);
    Bind(wxEVT_SCROLLWIN_BOTTOM, &BaseWidgetList::OnScroll, this);
    Bind(wxEVT_SCROLLWIN_LINEUP, &BaseWidgetList::OnScroll, this);
    Bind(wxEVT_SCROLLWIN_LINEDOWN, &BaseWidgetList::OnScroll, this);
    Bind(wxEVT_SCROLLWIN_PAGEUP, &BaseWidgetList::OnScroll, this);
    Bind(wxEVT_SCROLLWIN_PAGEDOWN, &BaseWidgetList::OnScroll, this);
    Bind(wxEVT_SCROLLWIN_THUMBTRACK, &BaseWidgetList::OnScroll, this);
    Bind(wxEVT_SCROLLWIN_THUMBRELEASE, &BaseWidgetList::OnScroll, this);

    Bind(wxEVT_SIZE, &BaseWidgetList::OnSize, this);
}

//////////////////////////////////////////////////////////////////////////

void BaseWidgetList::OnScroll(wxScrollWinEvent& evt)
{
    evt.Skip();

    // Throttle refreshes so we don't init/deinit widgets on every tiny thumb movement
    if (!m_refreshPending)
    {
        m_refreshPending = true;
        CallAfter([this]()
            {
                m_refreshPending = false;
                RefreshVisibleWidgets();
            });
    }
}

void BaseWidgetList::OnSize(wxSizeEvent& event)
{
    event.Skip();

    // Throttle refreshes so we don't init/deinit widgets on every tiny thumb movement
    if (!m_refreshPending)
    {
        m_refreshPending = true;
        CallAfter([this]()
            {
                m_refreshPending = false;
                RefreshVisibleWidgets();
            });
    }
}

void BaseWidgetList::OnWidgetClicked(wxCommandEvent& evt)
{
    const wxUIntPtr clickedId = wxPtrToUInt(evt.GetClientData());
    auto it = m_widgetIdtoItem.find(clickedId);
    if (it == m_widgetIdtoItem.end())
        return;

    std::shared_ptr<BaseWidget> clicked = it->second;

    // Check if clicking the same widget (toggle current)
    if (m_selectedWidget == clicked)
    {
        if (!clicked->IsSelected())
            m_selectedWidget = nullptr;
    }
    else
    {
        // Deselect previous
        if (m_selectedWidget)
            m_selectedWidget->SetSelected(false);

        m_selectedWidget = clicked;
    }

    evt.Skip();
}

void BaseWidgetList::OnToggleMetaPanel(wxCommandEvent& evt)
{
    const wxUIntPtr clickedId = wxPtrToUInt(evt.GetClientData());
    auto it = m_widgetIdtoItem.find(clickedId);
    if (it == m_widgetIdtoItem.end())
        return;


    auto orderIt = std::find(m_order.begin(), m_order.end(), clickedId);
    if (orderIt != m_order.end())
    {
        size_t index = std::distance(m_order.begin(), orderIt);

        // Remove corresponding height
        if (index < m_itemHeights.size())
        {
            m_itemHeights[index] = it->second->GetPreferredHeight();
        }
    }

    RecalculateHeightsAndPositions();
    RefreshVisibleWidgets();
    evt.Skip();
}

//////////////////////////////////////////////////////////////////////////

void BaseWidgetList::RemoveSelectedWidget()
{
    if (!m_selectedWidget)
        return;

    m_selectedWidget->Deinitialize();

    const wxUIntPtr toRemoveId = m_itemToWidgetId.at(m_selectedWidget);

    m_itemToWidgetId.erase(m_selectedWidget);

    m_widgetIdtoItem.erase(toRemoveId);

    RemoveSelectedWidgetImpl(m_selectedWidget);
}

void BaseWidgetList::RemoveAllWidgets()
{
	for (auto& [w, _] : m_itemToWidgetId) w->Deinitialize();

    m_itemToWidgetId.clear();
    m_widgetIdtoItem.clear();
    m_order.clear();
    m_itemHeights.clear();
    m_cumulativeY.clear();
    m_selectedWidget = nullptr;

    RemoveAllWidgetsImpl();
}

void BaseWidgetList::UnselectWidget()
{
    // Deselect
    if (m_selectedWidget)
        m_selectedWidget->SetSelected(false);

    m_selectedWidget = nullptr;
}

/////////////////////////////////////////////////////////////////////////

void BaseWidgetList::SetWidgets(const std::vector<std::shared_ptr<BaseWidget>>& widgets)
{
    CallAfter([this, widgets]()
        {
            // Deinitialize any existing widgets first
            for (auto& [w, _] : m_itemToWidgetId) w->Deinitialize();
            m_itemToWidgetId.clear();
            m_widgetIdtoItem.clear();
            m_order.clear();
            m_itemHeights.clear();
            m_cumulativeY.clear();

            for (const auto& w : widgets)
            {
                const wxUIntPtr widgetId = w->GetWidgetId();

                m_itemToWidgetId[w] = widgetId;

                m_widgetIdtoItem[widgetId] = w;

                m_order.push_back(widgetId);

                m_itemHeights.push_back(w->GetPreferredHeight());

                if (w->HasMetadataPanel())
                {
                    w->Bind(GUI_TOGGLE_META_WIDGET, &BaseWidgetList::OnToggleMetaPanel, this);
                }
            }

            RecalculateHeightsAndPositions();
            RefreshVisibleWidgets();
        });
}

void BaseWidgetList::UpdateVirtualSize()
{
    if (m_cumulativeY.empty())
    {
        SetVirtualSize(0, 0);
        return;
    }

    int virtWidth = std::max(GetClientSize().GetWidth(), 100);  // never 0
    int virtHeight = m_cumulativeY.back();

    SetVirtualSize(virtWidth, virtHeight);
}

void BaseWidgetList::RecalculateHeightsAndPositions()
{
    m_cumulativeY.resize(m_itemToWidgetId.size() + 1);
    m_cumulativeY[0] = 0;
    for (size_t i = 0; i < m_itemToWidgetId.size(); ++i)
    {
        m_cumulativeY[i + 1] = m_cumulativeY[i] + m_itemHeights[i];
    }

    UpdateVirtualSize();
}

void BaseWidgetList::RefreshVisibleWidgets()
{
    if (m_order.empty())
        return;

    // Get current scroll position in virtual pixels
    int scrollUnitsX, scrollUnitsY;
    GetViewStart(&scrollUnitsX, &scrollUnitsY);

    wxSize pixelsPerUnit;
    GetScrollPixelsPerUnit(&pixelsPerUnit.x, &pixelsPerUnit.y);
    int viewTop = scrollUnitsY * pixelsPerUnit.GetHeight();
    int clientH = GetClientSize().GetHeight();
    int viewBottom = viewTop + clientH;

    const size_t margin = 10;

    size_t first = GetFirstVisibleIndex(viewTop);
    size_t last = GetLastVisibleIndex(viewBottom);

    first = (first > margin) ? first - margin : 0;
    last = std::min(last + margin, m_order.size() - 1);

    Freeze();

    // 1. De-initialize widgets that are now completely out of view
    for (size_t i = 0; i < m_order.size(); ++i)
    {
        if ((i < first || i > last))
            m_widgetIdtoItem.at(m_order[i])->Deinitialize();
    }

    // 2. Initialize / reposition the visible widgets
    for (size_t i = first; i <= last; ++i)
    {
        std::shared_ptr<BaseWidget> w = m_widgetIdtoItem.at(m_order[i]);
        w->Initialize(false);

        // Convert virtual to physical client coordinates
        int physX, physY;
        CalcScrolledPosition(0, m_cumulativeY[i], &physX, &physY);

        wxPanel* widgetPanel = w->GetWidgetPanel();
        widgetPanel->SetPosition(wxPoint(physX, physY));
        widgetPanel->SetSize(GetClientSize().GetWidth(), m_itemHeights[i]);

        widgetPanel->Show(true);
    }

    Thaw();
}

size_t BaseWidgetList::GetFirstVisibleIndex(int viewTop) const
{
    if (m_cumulativeY.empty())
        return 0;

    if (viewTop <= 0)
        return 0;

    // Find the smallest k such that m_cumulativeY[k] > viewTop
    // (for example, the first item whose END > viewTop)
    auto it = std::upper_bound(m_cumulativeY.begin(), m_cumulativeY.end(), viewTop);
    size_t k = it - m_cumulativeY.begin();

    if (k == 0)
        return 0;

    // Clamp in case viewTop is beyond the total height
    return std::min(k - 1, m_order.size() - 1);
}

size_t BaseWidgetList::GetLastVisibleIndex(int viewBottom) const
{
    if (m_cumulativeY.empty())
        return 0;

    // Find the smallest k such that m_cumulativeY[k] >= viewBottom
    // for example, the first item whose START >= viewBottom)
    auto it = std::lower_bound(m_cumulativeY.begin(), m_cumulativeY.end(), viewBottom);
    size_t k = it - m_cumulativeY.begin();

    if (k == 0)
        return 0;

    // Clamp so we never go past the last real item
    return std::min(k - 1, m_order.size() - 1);
}

void BaseWidgetList::AppendWidget(const std::shared_ptr<BaseWidget>& widget)
{
    const wxUIntPtr widgetId = widget->GetWidgetId();

    if (widget->HasMetadataPanel())
    {
        widget->Bind(GUI_TOGGLE_META_WIDGET, &BaseWidgetList::OnToggleMetaPanel, this);
    }

    m_itemToWidgetId[widget] = widgetId;

    m_widgetIdtoItem[widgetId] = widget;

    m_order.push_back(widgetId);

    m_itemHeights.push_back(widget->GetPreferredHeight());

    RecalculateHeightsAndPositions();
    RefreshVisibleWidgets();
}

void BaseWidgetList::RemoveWidget(const std::shared_ptr<BaseWidget>& widget)
{
    auto it = m_itemToWidgetId.find(widget);
    if (it == m_itemToWidgetId.end())
        return;

    wxUIntPtr widgetId = it->second;

    if (it->first->HasMetadataPanel())
    {
        it->first->Unbind(GUI_TOGGLE_META_WIDGET, &BaseWidgetList::OnToggleMetaPanel, this);
    }

    // Remove from widget -> id map
    m_itemToWidgetId.erase(it);

    // Remove from id -> widget map
    m_widgetIdtoItem.erase(widgetId);

    // Find index in order vector
    auto orderIt = std::find(m_order.begin(), m_order.end(), widgetId);
    if (orderIt != m_order.end())
    {
        size_t index = std::distance(m_order.begin(), orderIt);

        // Remove from order
        m_order.erase(orderIt);

        // Remove corresponding height
        if (index < m_itemHeights.size())
            m_itemHeights.erase(m_itemHeights.begin() + index);
    }

    widget->Deinitialize();

    RecalculateHeightsAndPositions();
    RefreshVisibleWidgets();
}