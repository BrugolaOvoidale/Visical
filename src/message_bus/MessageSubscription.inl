#pragma once
#include "MessageSubscription.hpp"


template<typename MessageType>
    requires std::derived_from<MessageType, Message>
MessageSubscription<MessageType>& MessageSubscription<MessageType>::filterBy(std::function<bool(const MessageType&)> predicate)
{
    std::unique_lock lock(mutex_);

    customFilter_ = std::move(predicate);

    return *this;
}

template<typename MessageType>
    requires std::derived_from<MessageType, Message>
MessageSubscription<MessageType>& MessageSubscription<MessageType>::unsetCustomFilter()
{
    std::unique_lock lock(mutex_);

    customFilter_ = std::nullopt;

    return *this;
}

template<typename MessageType>
    requires std::derived_from<MessageType, Message>
std::optional<std::function<bool(const MessageType&)>> MessageSubscription<MessageType>::getCustomFilter() const
{
    std::shared_lock lock(mutex_);
    return customFilter_;
}

template<typename MessageType>
    requires std::derived_from<MessageType, Message>
bool MessageSubscription<MessageType>::shouldAccept(const MessageType& event) const
{
    std::shared_lock lock(mutex_);

    if (customFilter_.has_value() && !(customFilter_.value())(event))
        return false;

    return true;
}