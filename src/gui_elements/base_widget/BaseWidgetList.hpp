#pragma once
#include <memory>
#include <wx/scrolwin.h>
#include <unordered_map>


// Forward declarations
class BaseWidget;


/**
 * @brief An abstract scrollable container designed for high-performance widget management.
 *
 * BaseWidgetList implements a virtualized scrolling pattern where only the widgets
 * currently visible in the viewport are processed/refreshed. It manages a collection
 * of BaseWidget objects and handles their spatial layout dynamically.
 */
class BaseWidgetList : public wxScrolledWindow
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    virtual ~BaseWidgetList() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /** @brief Triggers the removal of the currently highlighted widget. */
    void RemoveSelectedWidget();

    /** @brief Clears all widgets from the list and resets scrolling parameters. */
    void RemoveAllWidgets();

    /** @brief Deselects the current widget, clearing the visual selection state. */
    void UnselectWidget();

protected:
    /**
     * @brief Protected constructor to ensure this class is only instantiated via a derived class.
     * @param parent The parent wxWindow.
     * @param winid Window identifier.
     * @param pos Window position.
     * @param size Window size.
     * @param style Scroll styles.
     * @param name Window name.
     */
    BaseWidgetList(
        wxWindow* parent,
        wxWindowID winid = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxVSCROLL | wxSIMPLE_BORDER,
        const wxString& name = wxASCII_STR(wxPanelNameStr)
    );

    // Replaces the current set of widgets with a new collection.
    void SetWidgets(const std::vector<std::shared_ptr<BaseWidget>>& widgets);

    // Updates the scrollable area based on the cumulative height of all widgets.
    void UpdateVirtualSize();

    // Computes Y-offsets for every widget. Call this after adding/removing or resizing items.
    void RecalculateHeightsAndPositions();

    // Redraws/Updates only the widgets within the current scroll viewport.
    void RefreshVisibleWidgets();

    // Finds the index of the first widget visible at the given pixel offset.
    size_t GetFirstVisibleIndex(int viewTop) const;

    // Finds the index of the last widget visible at the given pixel offset.
    size_t GetLastVisibleIndex(int viewBottom) const;

    // Adds a new widget to the end of the list.
    void AppendWidget(const std::shared_ptr<BaseWidget>& widget);

    // Removes a specific widget instance from the internal maps and layout.
    void RemoveWidget(const std::shared_ptr<BaseWidget>& toRemoveWidget);

    // Derived class hook to handle logic when a selected widget is removed.
    virtual void RemoveSelectedWidgetImpl(const std::shared_ptr<BaseWidget>& baseWidget) = 0;

    // Derived class hook to handle data cleanup when the list is cleared.
    virtual void RemoveAllWidgetsImpl() = 0;


    //-----------------------------------------------------------------------------
    // Events
    //-----------------------------------------------------------------------------

    void OnScroll(wxScrollWinEvent& evt);

    void OnSize(wxSizeEvent& evt);

    void OnWidgetClicked(wxCommandEvent& evt);

    void OnToggleMetaPanel(wxCommandEvent& evt);

protected:
    // The currently selected widget instance. May be nullptr.
    std::shared_ptr<BaseWidget> m_selectedWidget{ nullptr };

private:
    // Individual height of each widget in pixels.
    std::vector<int> m_itemHeights;

    // Pre-calculated Y-positions for fast lookup during scrolling.
    std::vector<int> m_cumulativeY;

    // Flag to prevent redundant refresh calls.
    bool m_refreshPending{ false };

    // The display order of widget Ids.
    std::vector<wxUIntPtr> m_order;

    // Map of shared pointers via wxWidgets Id
    std::unordered_map<wxUIntPtr, std::shared_ptr<BaseWidget>> m_widgetIdtoItem;

    // Reverse map to find the Id associated with a shared pointer.
    std::unordered_map<std::shared_ptr<BaseWidget>, wxUIntPtr> m_itemToWidgetId;
};