#include "BaseControllerLogger.hpp"
#include <wx/app.h>
#include <gui_elements/logger/LoggerView.hpp>
#include <message_bus/types/MessageTask.hpp>


BaseControllerLogger::BaseControllerLogger(
	LoggerView* view,
	size_t bufferSize,
	size_t messageBufferSize)
	: MessageLogger(bufferSize, messageBufferSize),
	view_(view)
{
	if (!view_)
		throw std::invalid_argument("view is nullptr");

	view_->Bind(EVT_SELECT_MESSAGE, &BaseControllerLogger::OnSelectMessage, this);
}

////////////////////////////////////////////////////////////

void BaseControllerLogger::appendLogs(Log logs)
{
	wxTheApp->CallAfter([this, logs]() {
		view_->UpdateLogs(logs);
		});

	Logger::appendLogs(std::move(logs));
}

void BaseControllerLogger::appendLogs(std::vector<Log> logs)
{
	wxTheApp->CallAfter([this, logs]() {
		view_->UpdateLogs(logs);
		});

	Logger::appendLogs(std::move(logs));
}

void BaseControllerLogger::appendMessage(const Message& msg)
{
	if (const MessageTask* msgTsk = dynamic_cast<const MessageTask*>(&msg))
	{
		wxTheApp->CallAfter([this, msgTsk = *msgTsk]() {

			view_->UpdateMessage(msgTsk);

			});

	}
	else
	{
		wxTheApp->CallAfter([this, msg]() {

			view_->UpdateMessage(msg);

			});

	}

	MessageLogger::appendMessage(msg);
}

////////////////////////////////////////////////////////////

void BaseControllerLogger::DoSelectMessage(const wxCommandEvent& event)
{
	std::shared_ptr<Message> msg = getMessage(event.GetInt());
	if (!msg)
		return;

	view_->UpdateMessageLogs(msg->getLogs());
	view_->UpdateMessageContext(msg->context().getAttributes());
}

void BaseControllerLogger::OnSelectMessage(const wxCommandEvent& event)
{
	DoSelectMessage(event);
}