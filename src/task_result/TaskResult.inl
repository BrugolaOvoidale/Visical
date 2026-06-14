#pragma once
#include "TaskResult.hpp"


template<typename Payload>
TaskResultP<Payload>::TaskResultP(std::optional<Payload> p, std::vector<Log> log)
    : payload_(std::move(p)), log_(std::move(log))
{
}

template<typename Payload>
TaskResultP<Payload>::TaskResultP(std::optional<Payload> p, Log log)
    : payload_(std::move(p)), log_({ std::move(log) })
{
}

template<typename Payload>
TaskResultP<Payload>::TaskResultP(Payload p, std::vector<Log> log)
    : payload_(std::move(p)), log_(std::move(log))
{
}

template<typename Payload>
TaskResultP<Payload>::TaskResultP(Payload p, Log log)
    : payload_(std::move(p)), log_({ std::move(log) })
{
}

/////////////////////////////////////////////////

template<typename Payload>
const std::optional<Payload>& TaskResultP<Payload>::tryGetPayload() const
{
    return payload_;
}

template<typename Payload>
const Payload& TaskResultP<Payload>::getPayload() const
{
    return payload_.value();
}

template<typename Payload>
Payload TaskResultP<Payload>::takePayload() const
{
    return std::move(payload_.value());
}

template<typename Payload>
bool TaskResultP<Payload>::isSuccess() const
{
    return payload_.has_value();
}

template<typename Payload>
bool TaskResultP<Payload>::hasLog() const
{
    return !log_.empty();
}

template<typename Payload>
const std::vector<Log>& TaskResultP<Payload>::getLogs() const
{
    return log_;
}

template<typename Payload>
std::vector<Log> TaskResultP<Payload>::takeLogs()
{
    return std::move(log_);
}