#pragma once
#include "MessageTaskP.hpp"


template<typename T>
inline MessageTaskP<T>::MessageTaskP(
    const IMessageTag& tag,
    SourceId sourceId,
    MessageContext ctx,
    std::vector<Log> logs)
	: MessageTask(tag, sourceId, std::move(ctx), MessageTask::TaskStatus::PENDING, std::move(logs)),
    payload_(std::nullopt)
{
}

template<typename T>
inline MessageTaskP<T>::MessageTaskP(
    const IMessageTag& tag,
    SourceId sourceId,
    MessageContext ctx,
    MessageTask::TaskStatus s,
    std::optional<T> p,
    std::vector<Log> logs)
    : MessageTask(tag, sourceId, std::move(ctx), s, std::move(logs)),
    payload_(std::move(p))
{
}

template<typename T>
inline std::shared_ptr<Message> MessageTaskP<T>::clone() const
{
    return std::make_shared<MessageTaskP<T>>(*this);
}

//////////////////////////////////////////////

template<typename T>
inline MessageTaskP<T> MessageTaskP<T>::rejected(
    const IMessageTag& tag,
    std::vector<Log> logs,
    SourceId sourceId,
    MessageContext ctx)
{
    return { tag, sourceId, std::move(ctx), MessageTask::TaskStatus::REJECTED, std::nullopt, std::move(logs) };
}

template<typename T>
inline MessageTaskP<T> MessageTaskP<T>::rejected(
    const IMessageTag& tag,
    Log logs,
    SourceId sourceId,
    MessageContext ctx)
{
    return rejected(tag, std::vector<Log>{ std::move(logs) }, sourceId, std::move(ctx));
}

template<typename T>
inline MessageTaskP<T> MessageTaskP<T>::pending(
    const IMessageTag& tag,
    SourceId sourceId,
    MessageContext ctx,
    std::vector<Log> logs)
{
    return { tag, sourceId, std::move(ctx), MessageTask::TaskStatus::PENDING, std::nullopt, std::move(logs) };
}

template<typename T>
inline MessageTaskP<T> MessageTaskP<T>::pending(
    const IMessageTag& tag,
    SourceId sourceId,
    MessageContext ctx,
    Log logs)
{
    return pending(tag, sourceId, std::move(ctx), std::vector<Log>{std::move(logs)});
}

template<typename T>
inline MessageTaskP<T> MessageTaskP<T>::started(
    const IMessageTag& tag,
    SourceId sourceId,
    MessageContext ctx,
    std::vector<Log> logs)
{
    return { tag, sourceId, std::move(ctx), MessageTask::TaskStatus::STARTED, std::nullopt, std::move(logs) };
}

template<typename T>
inline MessageTaskP<T> MessageTaskP<T>::started(
    const IMessageTag& tag,
    SourceId sourceId,
    MessageContext ctx,
    Log logs)
{
    return started(tag, sourceId, std::move(ctx), std::vector<Log>{std::move(logs)});
}

template<typename T>
inline MessageTaskP<T> MessageTaskP<T>::success(
    const IMessageTag& tag,
    T payload,
    SourceId sourceId,
    MessageContext ctx,
    std::vector<Log> logs)
{
    return { tag, sourceId, std::move(ctx), MessageTask::TaskStatus::SUCCESS, std::move(payload), std::move(logs) };
}

template<typename T>
inline MessageTaskP<T> MessageTaskP<T>::success(
    const IMessageTag& tag,
    T payload,
    SourceId sourceId,
    MessageContext ctx,
    Log logs)
{
    return started(tag, sourceId, std::move(ctx), std::vector<Log>{std::move(logs)});
}

template<typename T>
inline MessageTaskP<T> MessageTaskP<T>::fail(
    const IMessageTag& tag,
    std::vector<Log> logs,
    SourceId sourceId,
    MessageContext ctx)
{
    return { tag, sourceId, std::move(ctx), MessageTask::TaskStatus::FAIL, std::nullopt, std::move(logs) };
}

template<typename T>
inline MessageTaskP<T> MessageTaskP<T>::fail(
    const IMessageTag& tag,
    Log logs,
    SourceId sourceId,
    MessageContext ctx)
{
    return fail(tag, std::vector<Log>{ std::move(logs) }, sourceId, std::move(ctx));
}

//////////////////////////////////////////////

template<typename T>
inline const std::optional<T>& MessageTaskP<T>::tryGetPayload() const
{
    return payload_;
}

template<typename T>
inline const T& MessageTaskP<T>::getPayload() const
{
    return payload_.value();
}

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

template<typename T>
MessageTaskScopedP<T>::MessageTaskScopedP(
    const IMessageTag& messageTag,
    const std::shared_ptr<MessageDispatcher>& messageBus,
    SourceId sourceId,
    MessageContext ctx,
    std::vector<Log> logs)
    : messageTag_(&messageTag)
{
    messageBus_ = messageBus;
    sourceId_ = sourceId;
    ctx_ = std::move(ctx);
    status_ = MessageTask::TaskStatus::PENDING;
    finalized_ = false;
	messageBus_->publish(MessageTaskP<T>::pending(messageTag, sourceId_, ctx_, std::move(logs)));
}

template<typename T>
inline MessageTaskScopedP<T>::MessageTaskScopedP(MessageTaskScopedP&& other) noexcept
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

template<typename T>
inline MessageTaskScopedP<T>::~MessageTaskScopedP()
{
    if (!finalized_ && status_ != MessageTask::TaskStatus::SUCCESS && status_ != MessageTask::TaskStatus::FAIL)
    {
        messageBus_->publish(
            MessageTaskP<T>::fail(
                *messageTag_,
                { "Message id " + std::to_string(sourceId_.get()) + ": operation incomplete", Log::Level::LVL_WARNING },
                sourceId_,
                ctx_
            )
        );
    }
}

template<typename T>
inline MessageTaskScopedP<T>& MessageTaskScopedP<T>::operator=(MessageTaskScopedP<T>&& other) noexcept
{
    if (this != &other)
    {
        // Finalize current instance if needed
        if (!finalized_ && status_ != MessageTask::TaskStatus::SUCCESS && status_ != MessageTask::TaskStatus::FAIL)
        {
            messageBus_->publish(
                MessageTaskP<T>::fail(
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

template<typename T>
inline void MessageTaskScopedP<T>::start(std::vector<Log> logs)
{
    if (status_ == MessageTask::TaskStatus::PENDING)
    {
        messageBus_->publish(MessageTaskP<T>::started(*messageTag_, sourceId_, ctx_, std::move(logs)));
        status_ = MessageTask::TaskStatus::STARTED;
    }
}

template<typename T>
inline void MessageTaskScopedP<T>::start(Log logs)
{
    start(std::vector<Log>{std::move(logs)});
}

template<typename T>
inline void MessageTaskScopedP<T>::complete(T payload, std::vector<Log> logs)
{
    if (finalized_)
        return;

    messageBus_->publish(MessageTaskP<T>::success(*messageTag_, std::move(payload), sourceId_, ctx_, std::move(logs)));
    status_ = MessageTask::TaskStatus::SUCCESS;
    finalized_ = true;
}

template<typename T>
inline void MessageTaskScopedP<T>::complete(T payload, Log logs)
{
    complete(std::vector<Log>{std::move(logs)});
}

template<typename T>
inline void MessageTaskScopedP<T>::abort(std::vector<Log> logs)
{
    if (finalized_)
        return;

    messageBus_->publish(MessageTaskP<T>::fail(*messageTag_, std::move(logs), sourceId_, ctx_));
    status_ = MessageTask::TaskStatus::FAIL;
    finalized_ = true;
}

template<typename T>
inline void MessageTaskScopedP<T>::abort(Log logs)
{
    abort(std::vector<Log>{std::move(logs)});
}

template<typename T>
inline bool MessageTaskScopedP<T>::abortIf(bool condition, std::vector<Log> logs)
{
    if (condition)
    {
        abort(std::move(logs));
        return true;
    }
    return false;
}

template<typename T>
inline bool MessageTaskScopedP<T>::abortIf(bool condition, Log logs)
{
    if (condition)
    {
        abort(std::move(logs));
        return true;
    }
    return false;
}