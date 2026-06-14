#pragma once
#include <vector>
#include <deque>
#include <memory>
#include <shared_mutex>
#include <mutex>
#include <atomic>
#include "Log.hpp"


/**
 * @brief A thread-safe circular buffer for managing application logs.
 *
 * This class stores a finite number of Log entries in a deque.
 * It uses a readers-writer lock (shared_mutex) to allow multiple concurrent
 * reads while ensuring exclusive access during log appends.
 */
class Logger
{
public:
	//-----------------------------------------------------------------------------
	// Constructor/Destructor
	//-----------------------------------------------------------------------------
	
	/**
	 * @brief Constructs a Logger with a fixed maximum buffer size.
	 * @param bufferSize The maximum number of logs to keep in memory.
	 */
	Logger(size_t bufferSize = 1000);

	virtual ~Logger() = default;


	//-----------------------------------------------------------------------------
	// Class operators
	//-----------------------------------------------------------------------------
	
	/**
	 * @brief Appends a single log entry to the buffer.
	 * 
	 * If the buffer exceeds its capacity, the oldest log entry is removed.
	 * 
	 * @param logs The log entry to add.
	 */
	virtual void appendLogs(Log logs);

	/**
	 * @brief Appends multiple log entries to the buffer.
	 * @param logs A vector of log entries to add.
	 */
	virtual void appendLogs(std::vector<Log> logs);

	/**
	 * @brief Retrieves all logs currently stored in the buffer.
	 * @return A vector containing the log entries.
	 */
	std::vector<Log> getLogs() const;

private:
	// Maximum capacity of the log deque.
	std::atomic<size_t> bufferSize_;

	// Protects logs_ from concurrent access.
	mutable std::shared_mutex logsMutex_;

	// Internal storage for log entries.
	std::deque<Log> logs_;
};
