#pragma once
#include <wx/frame.h>


/////////////////////////////////////////////////////

wxDECLARE_EVENT(EVT_SELECT_MESSAGE, wxCommandEvent);

/////////////////////////////////////////////////////


// Forward declarations
class wxDataViewEvent;
class wxDataViewListCtrl;
class Log;
class Message;
class MessageTask;
class MessageContext;


/**
 * @brief A diagnostic and log visualization frame based on wxWidgets.
 *
 * This frame displays split-pane views monitoring system logs, active messages,
 * contextual data, and specific task tracking. It acts as the presentation layer
 * for the logging subsystem.
 */
class LoggerView : public wxFrame
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    /**
     * @brief Constructs the LoggerView frame.
     * @param parent Pointer to the parent window.
     * @param id Window identifier..
     * @param title The text displayed in the frame's title bar.
     * @param startShowed If true, the window is visible immediately upon construction.
     * @param pos Initial window position.
     * @param size Initial window size.
     * @param style Window style flags
     * @param name The internal name of the window component.
     */
    LoggerView(
        wxWindow* parent,
        wxWindowID id,
        const wxString& title,
        bool startShowed = false,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxDEFAULT_FRAME_STYLE,
        const wxString& name = wxASCII_STR(wxFrameNameStr)
    );

    ~LoggerView() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /**
     * @brief Shows the frame and brings it to the front.
     */
    void Open();

    /**
     * @brief Appends or updates a single log entry in the primary log view.
     * @param logs The single log record data.
     */
    void UpdateLogs(const Log& logs);

    /**
     * @brief Batch updates the primary log view with multiple log entries.
     * @param logs A collection of log record objects.
     */
    void UpdateLogs(const std::vector<Log>& logs);

    /**
     * @brief Updates or appends a general tracking message row in the message panel.
     * @param msg The updated data payload of the message.
     */
    void UpdateMessage(const Message& msg);

    /**
     * @brief Updates or appends a tracking message associated specifically with an active task.
     * @param msg The updated task-level message data.
     */
    void UpdateMessage(const MessageTask& msg);

    /**
     * @brief Refreshes the detailed log subset related directly to the currently selected message.
     * @param logs Sub-collection of logs associated with a single tracking context.
     */
    void UpdateMessageLogs(const std::vector<Log>& logs);

    /**
     * @brief Refreshes the contextual environment variables/properties panel for a message.
     * @param msgCtx Property data making up the context.
     */
    void UpdateMessageContext(const MessageContext& msgCtx);

private:
    // Factory helper to instantiate and style the general logs viewer control.
    wxDataViewListCtrl* CreateLogsList(wxWindow* parent);

    // Factory helper to instantiate and style the active message tracking control.
    wxDataViewListCtrl* CreateMessagesList(wxWindow* parent);

    // Factory helper to instantiate and style the variable metadata inspector list.
    wxDataViewListCtrl* CreateMessageContextList(wxWindow* parent);

    // Low-level worker implementation to push structured log rows to a target view control.
    void DoUpdateLogs(const Log& logs, wxDataViewListCtrl* list);

    // Event handler triggered via UI control/button to clear the general logs panel.
    void OnClearLog(wxCommandEvent& event);

    // Event handler triggered via UI control/button to clear the message summary list.
    void OnClearMessages(wxCommandEvent& event);

    // Event handler triggered when a user changes row selection inside the messages view.
    void OnSelectMessage(wxDataViewEvent& event);

    // Captures intercepted frame closure requests to handle clean layout teardown safely.
    void OnClose(wxCloseEvent& event);

private:
    // UI component displaying absolute system log tracking.
    wxDataViewListCtrl* m_logsList;

    // UI component listing processes summaries.
    wxDataViewListCtrl* m_messagesList;

    // UI component displaying local logs tied solely to the active selection.
    wxDataViewListCtrl* m_msgLogsList;

    // UI component containing state metrics and contextual variables.
    wxDataViewListCtrl* m_msgCtxList;
};