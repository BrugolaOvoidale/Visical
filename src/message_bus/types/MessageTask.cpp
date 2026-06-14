#include "MessageTask.hpp"
#include "../MessageDispatcher.hpp"


MessageTask::MessageTask(
    const IMessageTag& tag,
    SourceId id,
    MessageContext ctx,
    TaskStatus s,
    std::vector<Log> logs)
    : Message(tag, id, std::move(ctx), std::move(logs)),
    status_(s)
{
}

std::shared_ptr<Message> MessageTask::clone() const
{
    return std::make_shared<MessageTask>(*this);
}

//////////////////////////////////////////////

MessageTask MessageTask::rejected(
    const IMessageTag& tag,
    std::vector<Log> logs,
    SourceId id,
    MessageContext ctx)
{
    return { tag, id, std::move(ctx), TaskStatus::REJECTED, std::move(logs) };
}

MessageTask MessageTask::rejected(
    const IMessageTag& tag,
    Log logs,
    SourceId id,
    MessageContext ctx)
{
    return rejected(tag, std::vector<Log>{ std::move(logs) }, id, std::move(ctx));
}

MessageTask MessageTask::pending(
    const IMessageTag& tag,
    SourceId id,
    MessageContext ctx,
    std::vector<Log> logs)
{
    return { tag, id, std::move(ctx), TaskStatus::PENDING, std::move(logs) };
}

MessageTask MessageTask::pending(
    const IMessageTag& tag,
    SourceId id,
    MessageContext ctx,
    Log logs)
{
    return pending(tag, id, std::move(ctx), std::vector<Log>{ std::move(logs) });
}

MessageTask MessageTask::started(
    const IMessageTag& tag,
    SourceId id,
    MessageContext ctx,
    std::vector<Log> logs)
{
    return { tag, id, std::move(ctx), TaskStatus::STARTED, std::move(logs) };
}

MessageTask MessageTask::started(
    const IMessageTag& tag,
    SourceId id,
    MessageContext ctx,
    Log logs)
{
    return started(tag, id, std::move(ctx), std::vector<Log>{ std::move(logs) });
}

MessageTask MessageTask::success(
    const IMessageTag& tag,
    SourceId id,
    MessageContext ctx,
    std::vector<Log> logs)
{
    return { tag, id, std::move(ctx), TaskStatus::SUCCESS, std::move(logs) };
}

MessageTask MessageTask::success(
    const IMessageTag& tag,
    SourceId id,
    MessageContext ctx,
    Log logs)
{
    return started(tag, id, std::move(ctx), std::vector<Log>{ std::move(logs) });
}

MessageTask MessageTask::fail(
    const IMessageTag& tag,
    std::vector<Log> logs,
    SourceId id,
    MessageContext ctx)
{
    return { tag, id, std::move(ctx), TaskStatus::FAIL, std::move(logs) };
}

MessageTask MessageTask::fail(
    const IMessageTag& tag,
    Log logs,
    SourceId id,
    MessageContext ctx)
{
    return fail(tag, std::vector<Log>{ std::move(logs) }, id, std::move(ctx));
}

//////////////////////////////////////////////

MessageTask::TaskStatus MessageTask::status() const
{
    return status_;
}


////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

MessageTaskScoped::MessageTaskScoped(
    const IMessageTag& messageTag,
    const std::shared_ptr<MessageDispatcher>& messageBus,
    SourceId sourceId,
    MessageContext ctx,
    std::vector<Log> logs)
    : messageTag_(&messageTag), messageBus_(messageBus), sourceId_(sourceId), ctx_(std::move(ctx))
{
    status_ = MessageTask::TaskStatus::PENDING;
    messageBus_->publish(MessageTask::pending(messageTag, sourceId_, ctx_, std::move(logs)));
}

MessageTaskScoped::MessageTaskScoped(MessageTaskScoped&& other) noexcept
    : messageBus_(std::move(other.messageBus_)),
    messageTag_(other.messageTag_),
    status_(other.status_),
    sourceId_(std::move(other.sourceId_)),
    ctx_(std::move(other.ctx_)),
    finalized_(other.finalized_)
{
    // Prevent the moved-from object from sending a fail on destruction
    other.finalized_ = true;
}

MessageTaskScoped::~MessageTaskScoped()
{
    if (!finalized_ && status_ != MessageTask::TaskStatus::SUCCESS && status_ != MessageTask::TaskStatus::FAIL)
    {
        messageBus_->publish(
            MessageTask::fail(
                *messageTag_,
                { "Message id " + std::to_string(sourceId_.get()) + ": operation incomplete", Log::Level::LVL_WARNING },
                sourceId_,
                ctx_
            )
        );
    }
}

MessageTaskScoped& MessageTaskScoped::operator=(MessageTaskScoped&& other) noexcept
{
    if (this != &other)
    {
        // Finalize current instance if needed
        if (!finalized_ &&
            status_ != MessageTask::TaskStatus::SUCCESS &&
            status_ != MessageTask::TaskStatus::FAIL)
        {
            messageBus_->publish(
                MessageTask::fail(
                    *messageTag_,
                    { "Message id " + std::to_string(sourceId_.get()) + ": operation incomplete", Log::Level::LVL_WARNING },
                    sourceId_,
                    ctx_
                )
            );
        }

        messageBus_ = std::move(other.messageBus_);
        messageTag_ = other.messageTag_;
        status_ = other.status_;
        sourceId_ = std::move(other.sourceId_);
        ctx_ = std::move(other.ctx_);
        finalized_ = other.finalized_;

        other.finalized_ = true;
    }
    return *this;
}

//////////////////////////////////////////////

void MessageTaskScoped::start(std::vector<Log> logs)
{
    if (status_ == MessageTask::TaskStatus::PENDING)
    {
        messageBus_->publish(MessageTask::started(*messageTag_, sourceId_, ctx_, std::move(logs)));
        status_ = MessageTask::TaskStatus::STARTED;
    }
}

void MessageTaskScoped::start(Log logs)
{
    start(std::vector<Log>{std::move(logs)});
}

void MessageTaskScoped::complete(std::vector<Log> logs)
{
    if (finalized_)
        return;

    messageBus_->publish(MessageTask::success(*messageTag_, sourceId_, ctx_, std::move(logs)));
    status_ = MessageTask::TaskStatus::SUCCESS;
    finalized_ = true;
}

void MessageTaskScoped::complete(Log logs)
{
    complete(std::vector<Log>{std::move(logs)});
}

void MessageTaskScoped::abort(std::vector<Log> logs)
{
    if (finalized_)
        return;

    messageBus_->publish(MessageTask::fail(*messageTag_, std::move(logs), sourceId_, ctx_));
    status_ = MessageTask::TaskStatus::FAIL;
    finalized_ = true;
}

void MessageTaskScoped::abort(Log logs)
{
    abort(std::vector<Log>{std::move(logs)});
}

bool MessageTaskScoped::abortIf(bool condition, std::vector<Log> logs)
{
    if (condition)
    {
        abort(std::move(logs));
        return true;
    }
    return false;
}

bool MessageTaskScoped::abortIf(bool condition, Log logs)
{
    if (condition)
    {
        abort(std::move(logs));
        return true;
    }
    return false;
}