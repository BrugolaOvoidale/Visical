#pragma once
#include <wx/window.h>


// Forward declarations
class wxActivityIndicator;
class wxStaticText;
class wxTopLevelWindowBase;


/**
 * @brief An overlay window designed to block user interaction and display a
 * loading visual over a parent window during long-running tasks.
 *
 * This class inherits from wxWindow to sit directly on top of a specific parent
 * window. It dynamically captures and reflects the parent's visibility and position changes
 * to ensure seamless alignment, and can toggle between a raw text message and a visual loading spinner.
 */
class BusyOverlay : public wxWindow
{
public:
    //-----------------------------------------------------------------------------
    // Constants
    //-----------------------------------------------------------------------------

    static inline const wxString DEFAULT_LOADING{ "Loading..." };

    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    /**
     * @brief Constructs a new BusyOverlay instance.
     * @param parent The window over which this overlay will be drawn and centered.
     * @note The constructor handles retrieving the top-level parent window required for tracking bounds.
     */
    BusyOverlay(wxWindow* parent);

    ~BusyOverlay() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /**
     * @brief Toggles the overall visibility of the loading overlay.
     * @param show True to display and focus the overlay, false to hide it.
     */
    void ShowOverlay(bool show = true);

    /**
     * @brief Switches the overlay layout to a static text display mode.
     * @param txt The descriptive message to be displayed to the user.
     */
    void UseStaticText(const wxString& txt = DEFAULT_LOADING);

    /**
     * @brief Switches the overlay layout to a visual spinning element indicator mode.
     */
    void UseSpinner();

private:
    // Recalculates and adjusts the window position and size to match its target parent.
    void UpdatePosition();


    //-----------------------------------------------------------------------------
    // Events
    //-----------------------------------------------------------------------------

    // Handles rendering customized graphics or semi-transparent backgrounds onto the overlay.
    void OnPaint(wxPaintEvent& evt);

    // Responds to moving, resizing, or state updates affecting the parent window.
    void OnParentUpdate(wxEvent& evt);

    // Synchronizes visibility when the underlying target window is shown or hidden.
    void OnParentShow(wxShowEvent& evt);

    // Minimizes or restores the overlay cleanly alongside the parent frame state.
    void OnParentIconize(wxIconizeEvent& evt);

private:
    // The immediate target parent control window frame.
    wxWindow* m_parent;

    // The ancestor top-level window tracked for synchronization.
    wxTopLevelWindowBase* m_topParent;

    // Pointer to native platform loading animations controls.
    wxActivityIndicator* m_spinner;

    // Label rendering text info elements when in text display layout mode.
    wxStaticText* m_text{ nullptr };

    // Tracks the logical requested display visibility state.
    bool m_showOverlay{ false };

    // Flag tracking if the active graphic element is a spinner vs text.
    bool m_useSpinner{ true };
};