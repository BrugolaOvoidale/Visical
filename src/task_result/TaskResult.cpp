#include "TaskResult.hpp"


TaskResult::TaskResult(bool success, std::vector<Log> log)
    : success_(success), log_(std::move(log))
{
}

TaskResult::TaskResult(bool success, Log log)
    : success_(success), log_({ std::move(log) })
{
}

/////////////////////////////////////////////////

bool TaskResult::isSuccess() const
{
    return success_;
}

bool TaskResult::hasLog() const
{
    return !log_.empty();
}

const std::vector<Log>& TaskResult::getLogs() const
{
    return log_;
}

std::vector<Log> TaskResult::takeLogs()
{
    return std::move(log_);
}


//////////////////////////////////////////////////////////////


TaskEnqueueResult::TaskEnqueueResult(Status status, std::vector<Log> log)
    : status_(status), log_(std::move(log))
{
}

TaskEnqueueResult::TaskEnqueueResult(Status status, Log log)
    : status_(status), log_({ std::move(log) })
{
}

/////////////////////////////////////////////////

TaskEnqueueResult::Status TaskEnqueueResult::getStatus() const
{
    return status_;
}

bool TaskEnqueueResult::hasLog() const
{
    return !log_.empty();
}

const std::vector<Log>& TaskEnqueueResult::getLogs() const
{
    return log_;
}

std::vector<Log> TaskEnqueueResult::takeLogs()
{
    return std::move(log_);
}