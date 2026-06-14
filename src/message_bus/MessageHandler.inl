#pragma once
#include "MessageHandler.hpp"


template<typename MessageType, typename Class>
    requires std::derived_from<MessageType, Message>
MemberHandler<MessageType, Class>::MemberHandler(
    Class* obj,
    void (Class::* method)(const MessageType&))
    : object_(obj), method_(method)
{
}

////////////////////////////////////////////////////

template<typename MessageType, typename Class>
    requires std::derived_from<MessageType, Message>
void MemberHandler<MessageType, Class>::call(const Message* msg)
{
    (object_->*method_)(*static_cast<const MessageType*>(msg));
}

template<typename MessageType, typename Class>
    requires std::derived_from<MessageType, Message>
bool MemberHandler<MessageType, Class>::matches(Class* obj, void (Class::* method)(const MessageType&)) const
{
    return obj == object_ && method == method_;
}


////////////////////////////////////////////////////


template<typename MessageType>
    requires std::derived_from<MessageType, Message>
LambdaHandler<MessageType>::LambdaHandler(std::function<void(const MessageType&)> fn)
    : fn_(std::move(fn))
{
}

template<typename MessageType>
    requires std::derived_from<MessageType, Message>
void LambdaHandler<MessageType>::call(const Message* msg)
{
    fn_(*static_cast<const MessageType*>(msg));
}