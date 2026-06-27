#pragma once
#include <wx/frame.h>
#include <wx/timer.h>


// Forward declarations
class wxPanel;
class wxStaticText;
class wxTimerEvent;


/**
 * @brief A custom tooltip that stays visible.
 *
 * This class also allow a global management system
 * where one instance can serve multiple target windows.
 */
class PersistentToolTip : public wxFrame
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    /**
     * @brief Constructor.
     * @param target The window that triggers this tooltip.
     * @param text The initial string to display.
     * @param isManual If true, show and hide must be manually managed.
     */
    PersistentToolTip(
        wxWindow* target,
        const wxString& text,
        bool isManual = false
    );

    ~PersistentToolTip();


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /**
     * @brief Registers a tooltip text for a specific window.
     * @param target The UI element to monitor.
     * @param text The message to show.
     */
    static void SetToolTip(
        wxWindow* target,
        const wxString& text
    );

    /** @brief Unregisters a window from the tooltip system. */
    static void RemoveToolTip(wxWindow* target);

    /** @brief Sets the delay in milliseconds before the tooltip appears. */
    void SetDelay(int ms);

    /** @brief Sets the duration in milliseconds the tooltip remains visible. */
    void SetAutoPop(int ms);

    /** @brief Sets the "reshow" delay for appearing quickly after a previous tooltip. */
    void SetReshow(int ms);

    /** @brief Sets the maximum width of the tooltip window (useful for wrapping text). */
    void SetMaxWidth(int width);

    /** @brief Set the target for this tooltip. If nullptr, show and hide must be manually managed. */
    void SetManual(bool isManual);

    /** @brief Gets the current maximum width. Returns -1 if no limit is set. */
    int GetMaxWidth() const;

    /** @brief Updates the text of the current tooltip. */
    void SetText(const wxString& text);

    /**
     * @brief Manually displays the tooltip at a specific screen position.
     * @param screenPos Absolute coordinates on the screen.
     * @param text The text to display.
     */
    void ShowAt(const wxPoint& screenPos, const wxString& text);

    /** @brief Dismiss tooltip now.*/
    void DismissNow();

private:
    void BuildUI();

    bool IsEnabled() const;

    int GetDelay() const;

    int GetAutoPop() const;

    int GetReshow() const;

    void ApplyMaxWidth();

    void ShowTooltip(const wxPoint& screenPos);

    void ShowNow(const wxPoint& screenPos);


    //-----------------------------------------------------------------------------
    // Events
    //-----------------------------------------------------------------------------

    static void sOnTargetEnter(wxMouseEvent& e);

    void OnTargetEnter(wxMouseEvent& e);

    void OnTargetLeave(wxMouseEvent& e);

    void OnDelayTimer(wxTimerEvent&);

    void OnAutoPopTimer(wxTimerEvent&);

private:
    enum { ID_DELAY_TIMER = wxID_HIGHEST + 1, ID_AUTOPOP_TIMER };

    /** @brief Maps target windows to their respective tooltip strings. */
    static inline std::unordered_map<wxWindow*, wxString> s_instances;

    /** @brief The single active tooltip instance used globally. */
    static inline PersistentToolTip* s_globalToolTip{ nullptr };

    bool m_isInitialized{ false };

    wxWindow* m_target;

    wxPanel* m_panel;

    wxString m_text;

    wxStaticText* m_label{ nullptr };

    // Delay before showing.
    int m_delayMs;

    wxPoint m_delayedPos;

    // Time until auto-hide.
    int m_autoPopMs;

    // Shortened delay if moving between targets.
    int m_reshowMs;

    bool m_enabled;

    bool m_wasVisible;

    bool m_isManual;

    wxTimer m_delayTimer;

    wxTimer m_autoPopTimer;

    int m_maxWidth{ -1 };
};