#pragma once
#include "MessageP.hpp"


template<typename T>
MessageP<T>::MessageP(
    const IMessageTag& tag,
    T p,
    SourceId id,
    MessageContext ctx,
    std::vector<Log> logs)
    : Message(tag, id, std::move(ctx), std::move(logs)),
    payload_(std::move(p))
{
}

///////////////////////////////////////

template<typename T>
void MessageP<T>::setPayload(T p)
{
    payload_ = std::move(p);
}

template<typename T>
const T& MessageP<T>::getPayload() const
{
    return payload_;
}
