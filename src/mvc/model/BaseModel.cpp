#include "BaseModel.hpp"
#include <message_logger/MessageLogger.hpp>


BaseModel::BaseModel(const std::shared_ptr<MessageLogger>& logger) : logger_(logger)
{
}

BaseModel::~BaseModel()
{
    shutdown();
}

/////////////////////////////////////////////////////////////////////////

void BaseModel::shutdown()
{
}

void BaseModel::attachLogger(const std::shared_ptr<MessageLogger>& logger)
{
    logger_ = logger;
}

void BaseModel::detachLogger()
{
    logger_.reset();
}

std::shared_ptr<MessageLogger> BaseModel::getLogger() const
{
    return logger_;
}


/////////////////////////////////////////////////////////////////////////

void BaseModel::UpdateLogs(Log logs)
{
    if (!logger_)
        return;

    logger_->appendLogs(std::move(logs));
}

void BaseModel::UpdateLogs(std::vector<Log> logs)
{
    if (!logger_)
        return;

    if (logs.empty())
		return;

    logger_->appendLogs(std::move(logs));
}

void BaseModel::UpdateLogsMessage(const Message& msg)
{
    if (!logger_)
        return;

    logger_->appendMessage(msg);
}
