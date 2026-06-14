#include "Logger.hpp"


Logger::Logger(size_t bufferSize) : bufferSize_(bufferSize)
{

}

////////////////////////////////////////////////////////////

void Logger::appendLogs(Log logs)
{
	const size_t bSize = bufferSize_.load();

	if (bSize == 0)
		return;

	std::unique_lock lock(logsMutex_);

	if (logs_.size() == bSize)
	{
		logs_.pop_back();
	}

	logs_.push_back(std::move(logs));
}

void Logger::appendLogs(std::vector<Log> logs)
{
	std::unique_lock lock(logsMutex_);

	for (auto& l : logs)
	{
		if (logs.size() == bufferSize_)
		{
			logs_.pop_back();
		}

		logs_.push_back(std::move(l));
	}
}

std::vector<Log> Logger::getLogs() const
{
	std::shared_lock lock(logsMutex_);

	return { logs_.begin(), logs_.end() };
}
