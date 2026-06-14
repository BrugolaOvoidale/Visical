#include "Message.hpp"
#include "../MessageTag.hpp"


Message::Message(
    const IMessageTag& tag,
    SourceId id,
    MessageContext ctx,
    std::vector<Log> logs)
    : tag_(&tag),
    sourceId_(id),
    context_(std::move(ctx)),
    logs_(std::move(logs)),
    timestamp_(std::chrono::system_clock::now())
{
}

std::shared_ptr<Message> Message::clone() const
{
	return std::make_shared<Message>(*this);
}

///////////////////////////////////////

const IMessageTag& Message::tag() const
{
    return *tag_;
}

SourceId Message::sourceId() const
{
    return sourceId_;
}

const MessageContext& Message::context() const
{
    return context_;
}

std::chrono::system_clock::time_point Message::timestamp() const
{
    return timestamp_;
}

const std::vector<Log>& Message::getLogs() const
{
    return logs_;
}