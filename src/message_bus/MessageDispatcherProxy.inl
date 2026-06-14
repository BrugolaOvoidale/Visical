#pragma once
#include "MessageDispatcherProxy.hpp"


template<typename MessageType, typename Class>
    requires std::derived_from<MessageType, Message>
inline MessageSubscription<MessageType>& MessageDispatcherProxy::subscribe(
    const MessageTag<MessageType>& tag,
    void(Class::* method)(const MessageType&),
    Class* obj)
{
    return msgDispatcher_->subscribe(tag, method, obj);
}

template<typename MessageType, typename Callable>
    requires std::derived_from<MessageType, Message>
inline MessageSubscription<MessageType>& MessageDispatcherProxy::subscribe(
    const MessageTag<MessageType>& tag,
    Callable&& fn)
{
    return msgDispatcher_->subscribe(tag, std::forward<Callable>(fn));
}

template<typename MessageType, typename Class>
    requires std::derived_from<MessageType, Message>
inline void MessageDispatcherProxy::unsubscribe(
    const MessageTag<MessageType>& tag,
    void(Class::* method)(const MessageType&),
    Class* obj)
{
    return msgDispatcher_->unsubscribe(tag, method, obj);
}

//////////////////////////////////////////////////////////////////////

template<typename MessageType>
    requires std::derived_from<MessageType, Message>
inline void MessageDispatcherProxy::publish(const MessageType& message)
{
    return msgDispatcher_->publish(message);
}