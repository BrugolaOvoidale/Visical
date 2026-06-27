#pragma once
#include <wx/event.h>
#include <wx/timer.h>


/**
 * @brief A helper class to execute periodic callbacks within the wxWidgets UI thread.
 *
 * This class inherits from 'wxEvtHandler' and wraps a 'wxTimer'. It is specifically
 * designed to trigger a functional callback at regular intervals. This is ideal
 * for updating UI elements, refreshing displays, or polling data without
 * blocking the main event loop.
 */
class UiTickProxy : public wxEvtHandler
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    /**
     * @brief Constructs a proxy with a specific callback.
     * @param cb The function to be executed on every timer tick.
     */
    explicit UiTickProxy(std::function<void(void)> cb);

    ~UiTickProxy();


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /**
     * @brief Starts the periodic timer.
     * @param intervalMs The time between ticks in milliseconds.
     */
    void Start(int intervalMs);

    /** @brief Checks if the timer is currently active. */
    bool IsRunning() const;

    /** @brief Stops the timer. No further callbacks will be triggered. */
    void Stop();

private:
    // Internal event handler for wxTimerEvent.
    void OnTimer(wxTimerEvent&);

private:
    // The user-defined function to call.
    std::function<void(void)> callback_;

    //The underlying wxWidgets timer object.
    wxTimer timer_;
};
