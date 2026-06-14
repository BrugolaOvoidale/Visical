#pragma once
#include <memory>
#include <logger/Log.hpp>


// Forward declarations
class BaseControllerLogger;
class Message;


/**
 * @brief Base Controller class.
 *
 * BaseController serves as the foundation for all controller-like objects in the system.
 * It facilitates the attachment of a BaseControllerLogger to capture internal logs and
 * messages.
 */
class BaseController
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    BaseController(const std::shared_ptr<BaseControllerLogger>& logger = nullptr);

    ~BaseController() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /**
     * @brief Associates a logger with this model.
     * @param logger The logger instance to attach.
     */
    virtual void attachLogger(const std::shared_ptr<BaseControllerLogger>& logger);

    /** @brief Removes the current logger association. */
    virtual void detachLogger();

    /** @brief Returns the currently attached logger, or nullptr if none exists. */
    std::shared_ptr<BaseControllerLogger> getLogger() const;

protected:
    // Submits a single log entry to the attached logger.
    void UpdateLogs(Log logs);

    // Submits multiple log entries to the attached logger.
    void UpdateLogs(std::vector<Log> logs);

    // Submits a single message to the logger.
    void UpdateLogsMessage(const Message& msg);

private:
    // Shared reference to the logging system.
    std::shared_ptr<BaseControllerLogger> logger_;
};