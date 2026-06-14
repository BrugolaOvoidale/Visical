#pragma once
#include <vector>
#include <optional>
#include <logger/Log.hpp>


/**
 * @brief Represents the outcome of a simple task that does not return data.
 * 
 * Encapsulates a success/failure status and a collection of logs generated
 * during the task execution.
 */
class TaskResult
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    /** @brief Constructs a result with a success flag and optional log vector. */
    TaskResult(bool success = false, std::vector<Log> log = {});

    /** @brief Convenience constructor for a result with a single log entry. */
    TaskResult(bool success, Log log);

    ~TaskResult() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /** @brief Returns true if the task was completed successfully. */
    bool isSuccess() const;

    /** @brief Checks if there are any log messages present. */
    bool hasLog() const;

    /** @brief Returns a reference to the log collection. */
    const std::vector<Log>& getLogs() const;

    /**
     * @brief Moves the logs out of this result object.
     * @return A vector of logs. After this call, the internal log container is empty.
     */
    std::vector<Log> takeLogs();

private:
    // Success status of the task.
    bool success_;

    // Log entries collected during execution.
    std::vector<Log> log_;
};

//////////////////////////////////////////////////////////////

/**
* @brief    A template class for tasks that return a payload upon completion.
 *          Similar to TaskResult, but includes an optional payload of type Payload.
 *          Success is typically inferred by the presence of the payload.
 * @tparam  Payload The type of the data object returned by the task.
 */
template<typename Payload>
class TaskResultP
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    /** @brief Constructs with an optional payload and log vector. */
    TaskResultP(std::optional<Payload> p = std::nullopt, std::vector<Log> log = {});

    /** @brief Constructs with an optional payload and a single log entry. */
    TaskResultP(std::optional<Payload> p, Log log);

    /** @brief Constructs with a valid payload and optional log vector. */
    TaskResultP(Payload p, std::vector<Log> log = {});

    /** @brief Constructs with a valid payload and a single log entry. */
    TaskResultP(Payload p, Log log);

    ~TaskResultP() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /** @brief Returns the payload if it exists, otherwise std::nullopt. */
    const std::optional<Payload>& tryGetPayload() const;

    /**
     * @brief Accesses the payload directly.
     * 'std::optional' will throws if the payload is not set.
     */
    const Payload& getPayload() const;
    
    /**
     * @brief Moves the payload directly.
     * 'std::optional' will throws if the payload is not set.
     */
    Payload takePayload() const;

    /**
     * @brief Returns true if the task was completed successfully.
     * Convenience method for std::optional::has_value()
    */
    bool isSuccess() const;

    /** @brief Checks if there are any log messages present. */
    bool hasLog() const;

    /** @brief Returns a reference to the log collection. */
    const std::vector<Log>& getLogs() const;

    /**
     * @brief Moves the logs out of this result object.
     * @return A vector of logs. After this call, the internal log container is empty.
     */
    std::vector<Log> takeLogs();

private:
    // The task's output data.
    std::optional<Payload> payload_;

    // Log entries collected during execution.
    std::vector<Log> log_;
};

//////////////////////////////////////////////////////////////

/**
 * @brief Represents the specific result of attempting to queue a task for execution.
 */
class TaskEnqueueResult
{
public:
    //-----------------------------------------------------------------------------
    // Enums
    //-----------------------------------------------------------------------------

    /** @brief Status codes for task submission. */
    enum class Status {
        NO_ERRORS,          ///< Task request have not produced any errors
        ALREADY_ACTIVE,     ///< Already in progress (no-op)
        ALREADY_DONE,       ///< Already in desired state (no-op)
        REJECTED            ///< Cannot proceed
    };


    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    /** @brief Constructs an enqueue result with a status and optional log vector. */
    TaskEnqueueResult(Status status = Status::NO_ERRORS, std::vector<Log> log = {});

    /** @brief Convenience constructor for a status and a single log entry. */
    TaskEnqueueResult(Status status, Log log);

	~TaskEnqueueResult() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /** @brief Returns the queuing status. */
    TaskEnqueueResult::Status getStatus() const;

    /** @brief Checks if there are any log messages present. */
    bool hasLog() const;

    /** @brief Returns a reference to the log collection. */
    const std::vector<Log>& getLogs() const;

    /**
     * @brief Moves the logs out of this result object.
     * @return A vector of logs. After this call, the internal log container is empty.
     */
    std::vector<Log> takeLogs();

private:
    // The status of the enqueue attempt.
    TaskEnqueueResult::Status status_;

    // Log entries collected during enqueue attempt.
    std::vector<Log> log_;
};

#include "TaskResult.inl"