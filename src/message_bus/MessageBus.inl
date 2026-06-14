#pragma once
#include "MessageBus.hpp"


template<typename MessageType>
    requires std::derived_from<MessageType, Message>
template<typename Class>
inline MessageSubscription<MessageType>& MessageBus<MessageType>::subscribe(
    void(Class::* method)(const MessageType&),
    Class* obj)
{
    std::lock_guard<std::mutex> lock(mutex_);

    // Check if this exact subscription already exists
    for (const auto& entry : messages_)
    {
        if (auto handler = std::dynamic_pointer_cast<MemberHandler<MessageType, Class>>(entry.first))
        {
            if (handler->matches(obj, method))
            {
                // Already subscribed, just return the existing subscription handle
                return *entry.second;
            }
        }
    }

    // Not found, create new subscription
    messages_.emplace_back(
        std::make_shared<MemberHandler<MessageType, Class>>(obj, method),
        std::make_unique<MessageSubscription<MessageType>>()
    );

    // Return reference to the newly created subscription
    return *messages_.back().second;
}

template<typename MessageType>
    requires std::derived_from<MessageType, Message>
template<typename Callable>
inline MessageSubscription<MessageType>& MessageBus<MessageType>::subscribe(Callable&& fn)
{
    std::lock_guard<std::mutex> lock(mutex_);

    messages_.emplace_back(
        std::make_shared<LambdaHandler<MessageType>>(std::forward<Callable>(fn)),
        std::make_shared<MessageSubscription<MessageType>>()
    );

    return *messages_.back().second;
}

template<typename MessageType>
    requires std::derived_from<MessageType, Message>
template<typename Class>
inline void MessageBus<MessageType>::unsubscribe(
    void(Class::* method)(const MessageType&),
    Class* obj)
{
    std::lock_guard<std::mutex> lock(mutex_);

    messages_.erase(
        std::remove_if(messages_.begin(), messages_.end(),
            [&](const auto& pair)
            {
                auto memberHandler = std::dynamic_pointer_cast<MemberHandler<MessageType, Class>>(pair.first);
                return memberHandler && memberHandler->matches(obj, method);
            }),
        messages_.end());
}

template<typename MessageType>
    requires std::derived_from<MessageType, Message>
inline void MessageBus<MessageType>::publish(const MessageType& message)
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& [evtHandler, evtSub] : messages_)
    {
        if (evtSub->shouldAccept(message))
        {
            evtHandler->call(&message);
        }
    }
}