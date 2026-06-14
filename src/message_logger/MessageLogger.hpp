#pragma once
#include <logger/Logger.hpp>


// Forward declarations
class Message;


/**
 * @brief Specialization of 'Logger' class to aslo handle messages.
 */
class MessageLogger : public Logger
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    /**
     * @brief Constructs a Logger with a fixed maximum buffer size.
     * @param bufferSize The maximum number of logs to keep in memory.
     * @param messageBufferSize The maximum number of messages to keep in memory.
     */
    MessageLogger(
        size_t bufferSize = 1000,
        size_t messageBufferSize = 500
    );

    ~MessageLogger() = default;

    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /**
     * @brief Appends a single message entry to the buffer.
     *
     * If the buffer exceeds its capacity, the oldest message entry is removed.
     * @param msg The message entry to add.
     */
    virtual void appendMessage(const Message& msg);
    
    /**
     * @brief Get the message at specified index.
     * @param index Index of message entry in the queue.
     */
    std::shared_ptr<Message> getMessage(size_t index) const;

    /**
     * @brief Retrieves all messages currently stored in the buffer.
     * @return A vector containing the message entries.
     */
    std::vector<std::shared_ptr<Message>> getMessageLogs() const;

protected:
    // Maximum capacity of the message deque.
    std::atomic<size_t> messageBufferSize_;

private:
    // Protects messageLogs_ from concurrent access.
    mutable std::shared_mutex msgMutex_;

    // Internal storage for message entries.
    std::deque<std::shared_ptr<Message>> messageLogs_;
};
