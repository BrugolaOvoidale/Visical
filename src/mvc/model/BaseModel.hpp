#pragma once
#include <memory>
#include <logger/Log.hpp>


// Forward declarations
class MessageLogger;
class Message;


/**
 * @brief Base Model class.
 *
 * BaseModel serves as the foundation for all model-like objects in the system.
 * It facilitates the attachment of a MessageLogger to capture internal logs and
 * messages.
 */
class BaseModel
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    /**
     * @brief Constructs a BaseModel with an optional logger.
     * @param logger A shared pointer to a MessageLogger instance. Can be nullptr.
     */
    BaseModel(const std::shared_ptr<MessageLogger>& logger = nullptr);

    virtual ~BaseModel();


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /**
     * @brief Performs cleanup operations before the object is destroyed.
     * Subclasses should override this if they need to release their own resources.
     */
    virtual void shutdown();

    /**
     * @brief Associates a logger with this model.
     * @param logger The logger instance to attach.
     */
    virtual void attachLogger(const std::shared_ptr<MessageLogger>& logger);

    /** @brief Removes the current logger association. */
    virtual void detachLogger();

    /** @brief Returns the currently attached logger, or nullptr if none exists. */
    std::shared_ptr<MessageLogger> getLogger() const;

protected:
    // Submits a single log entry to the attached logger.
    void UpdateLogs(Log logs);

    // Submits multiple log entries to the attached logger.
    void UpdateLogs(std::vector<Log> logs);

    // Submits a single message to the logger.
    void UpdateLogsMessage(const Message& msg);

private:
    // Shared reference to the logging system.
    std::shared_ptr<MessageLogger> logger_;
};