#include "MessageLogger.hpp"
#include <message_bus/types/Message.hpp>


MessageLogger::MessageLogger(
	size_t bufferSize,
	size_t messageBufferSize)
	: Logger(bufferSize),
	messageBufferSize_(messageBufferSize)
{

}

/////////////////////////////////////////

void MessageLogger::appendMessage(const Message& msg)
{
	const size_t bSize = messageBufferSize_.load();

	if (bSize == 0)
		return;

	std::unique_lock lock(msgMutex_);

	if (messageLogs_.size() == bSize)
	{
		messageLogs_.pop_back();
	}

	messageLogs_.push_back(msg.clone());
}

std::shared_ptr<Message> MessageLogger::getMessage(size_t index) const
{
	std::shared_lock lock(msgMutex_);
	
	if (index >= messageLogs_.size())
		return nullptr;

	return messageLogs_[index];
}

std::vector<std::shared_ptr<Message>> MessageLogger::getMessageLogs() const
{
	std::shared_lock lock(msgMutex_);

	return { messageLogs_.begin(), messageLogs_.end() };
}