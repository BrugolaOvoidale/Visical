#pragma once
#include "ICameraProxy.hpp"


template<typename MessageType, typename Class>
    requires std::derived_from<MessageType, Message>
inline MessageSubscription<MessageType>& ICameraProxy::subscribe(
    const MessageTag<MessageType>& tag,
    void(Class::* method)(const MessageType&),
    Class* obj)
{
    return iCam_->subscribe(tag, method, obj);
}

template<typename MessageType, typename Callable>
    requires std::derived_from<MessageType, Message>
inline MessageSubscription<MessageType>& ICameraProxy::subscribe(
    const MessageTag<MessageType>& tag,
    Callable&& fn)
{
    return iCam_->subscribe(tag, std::forward<Callable>(fn));
}

template<typename MessageType, typename Class>
    requires std::derived_from<MessageType, Message>
inline void ICameraProxy::unsubscribe(
    const MessageTag<MessageType>& tag,
    void(Class::* method)(const MessageType&),
    Class* obj)
{
    return iCam_->unsubscribe(tag, method, obj);
}

template<typename Func, typename Class>
inline TaskEnqueueResult ICameraProxy::takeSnapshot(
    Func&& func,
    Class* obj,
    int timeout)
{
    return iCam_->takeSnapshot(func, obj, timeout);
}