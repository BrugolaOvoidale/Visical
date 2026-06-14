#pragma once
#include <wx/event.h>
#include <message_logger/MessageLogger.hpp>


// Forward declarations
class LoggerView;


/**
 * @brief A specialized logger that bridges message data to a LoggerView UI component.
 *
 * Inherits from MessageLogger to handle both standard log entries and high-level
 * application messages.
 */
class BaseControllerLogger : public MessageLogger
{
public:
	//-----------------------------------------------------------------------------
	// Constructor/Destructor
	//-----------------------------------------------------------------------------

	/**
	 * @brief Constructs a controller linked to a specific view.
	 * @param view Pointer to the UI view that will display the logs.
	 * @param bufferSize Maximum number of log entries to retain in memory.
	 * @param messageBufferSize Maximum number of high-level messages to retain.
	 * @throws std::invalid_argument if view is nullptr
	 */
	BaseControllerLogger(
		LoggerView* view,
		size_t bufferSize = 1000,
		size_t messageBufferSize = 500
	);

	BaseControllerLogger() = default;


	//-----------------------------------------------------------------------------
	// Class operators
	//-----------------------------------------------------------------------------

	// See Logger for details.
	void appendLogs(Log logs) override;

	// See Logger for details.
	void appendLogs(std::vector<Log> logs) override;

	// See MessageLogger for details.
	void appendMessage(const Message& msg) override;

private:
	// Handles the internal logic for selecting a message.
	void DoSelectMessage(const wxCommandEvent& event);

	// Catch the event triggered by the view when a message is selected.
	void OnSelectMessage(const wxCommandEvent& event);

private:
	// Pointer to the associated UI component.
	LoggerView* view_;
};
