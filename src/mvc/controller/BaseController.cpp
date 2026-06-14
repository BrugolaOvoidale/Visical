#include "BaseController.hpp"
#include "BaseControllerLogger.hpp"


BaseController::BaseController(const std::shared_ptr<BaseControllerLogger>& logger)
    : logger_(logger)
{
}

/////////////////////////////////////////////////////////////////////////

void BaseController::attachLogger(const std::shared_ptr<BaseControllerLogger>& logger)
{
	if (!logger) return;

    logger_ = logger;
}

void BaseController::detachLogger()
{
    logger_.reset();
}

std::shared_ptr<BaseControllerLogger> BaseController::getLogger() const
{
    return logger_;
}

/////////////////////////////////////////////////////////////////////////

void BaseController::UpdateLogs(Log logs)
{
    if (!logger_)
        return;

    logger_->appendLogs(std::move(logs));
}

void BaseController::UpdateLogs(std::vector<Log> logs)
{
    if (!logger_)
        return;

    if (logs.empty())
        return;

    logger_->appendLogs(std::move(logs));
}

void BaseController::UpdateLogsMessage(const Message& msg)
{
    if (!logger_)
        return;

    logger_->appendMessage(msg);
}