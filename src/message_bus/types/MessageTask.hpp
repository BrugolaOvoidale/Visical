#pragma once
#include "Message.hpp"
#include "../MessageDispatcher.hpp"


// Forward declarations
class MessageDispatcher;


/**
 * @brief A message representing the state and logs of a discrete task.
 * 
 * This class inherits from Message and adds state tracking (TaskStatus).
 * It is typically used to communicate the progress
 * or outcome of asynchronous operations across the system.
 */
class MessageTask : public Message
{
public:
    //-----------------------------------------------------------------------------
    // Enums
    //-----------------------------------------------------------------------------

    /** @brief Represents the lifecycle stages of a task. */
    enum class TaskStatus {
        REJECTED,   ///< Task was denied execution (e.g., invalid parameters).
        PENDING,    ///< Task entered the queue
        STARTED,    ///< Task execution is started.
        SUCCESS,    ///< Task completed successfully.
        FAIL        ///< Task failed during execution.
    };


    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    /**
     * @brief Detailed constructor for MessageTask.
     * @param tag The message identifier tag.
     * @param id The source identifier.
     * @param ctx Execution context/metadata.
     * @param s Initial status.
     * @param logs Initial collection of log entries.
     */
    MessageTask(
        const IMessageTag& tag,
        SourceId id = SourceId::none(),
        MessageContext ctx = {},
        TaskStatus s = TaskStatus::PENDING,
        std::vector<Log> logs = {}
    );

    std::shared_ptr<Message> clone() const override;

	~MessageTask() = default;


    //-----------------------------------------------------------------------------
    // Construction from factories
    //-----------------------------------------------------------------------------

    /** @brief Creates a REJECTED status message. */
    static MessageTask rejected(
        const IMessageTag& tag,
        std::vector<Log> logs,
        SourceId id = SourceId::none(),
        MessageContext ctx = {}
    );
    static MessageTask rejected(
        const IMessageTag& tag,
        Log logs,
        SourceId id,
        MessageContext ctx
    );


    /** @brief Creates a PENDING status message. */
    static MessageTask pending(
        const IMessageTag& tag,
        SourceId id = SourceId::none(),
        MessageContext ctx = {},
        std::vector<Log> logs = {}
    );
    static MessageTask pending(
        const IMessageTag& tag,
        SourceId id,
        MessageContext ctx,
        Log logs
    );


    /** @brief Creates a STARTED status message. */
    static MessageTask started(
        const IMessageTag& tag,
        SourceId id = SourceId::none(),
        MessageContext ctx = {},
        std::vector<Log> logs = {}
    );
    static MessageTask started(
        const IMessageTag& tag,
        SourceId id,
        MessageContext ctx,
        Log logs
    );

    
    /** @brief Creates a SUCCESS status message. */
    static MessageTask success(
        const IMessageTag& tag,
        SourceId id = SourceId::none(),
        MessageContext ctx = {},
        std::vector<Log> logs = {}
    );
    static MessageTask success(
        const IMessageTag& tag,
        SourceId id,
        MessageContext ctx,
        Log logs
    );


    /** @brief Creates a FAIL status message. */
    static MessageTask fail(
        const IMessageTag& tag,
        std::vector<Log> logs,
        SourceId id = SourceId::none(),
        MessageContext ctx = {}
    );
    static MessageTask fail(
        const IMessageTag& tag,
        Log logs,
        SourceId id,
        MessageContext ctx
    );


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /** @brief Returns the current status of the task. */
    TaskStatus status() const;

protected:
    TaskStatus status_;
};


template <>
struct EnumTraits<MessageTask::TaskStatus> {
    static constexpr std::array<std::pair<MessageTask::TaskStatus, const char*>, 5> values{ {
    {MessageTask::TaskStatus::PENDING, "PENDING"},
    {MessageTask::TaskStatus::REJECTED, "REJECTED"},
    {MessageTask::TaskStatus::STARTED, "STARTED"},
    {MessageTask::TaskStatus::SUCCESS, "SUCCESS"},
    {MessageTask::TaskStatus::FAIL, "FAIL"}
    } };
};


////////////////////////////////////////////////////////////////////


/**
 * @brief A RAII wrapper for task reporting.
 * 
 * This class automatically manages the lifecycle of a MessageTask.
 * - Upon construction, it sends a **PENDING** message.
 * - Calling start() sends a **STARTED** message.
 * - Calling complete() sends a **SUCCESS** message.
 * - **Crucially**: If the object is destroyed before complete() is called,
 * it automatically sends a **FAIL** message in the destructor.
 */
class MessageTaskScoped
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    /**
     * @brief Constructor. Immediately broadcasts a PENDING message via the bus.
     */
    MessageTaskScoped(
        const IMessageTag& messageTag,
        const std::shared_ptr<MessageDispatcher>& messageBus,
        SourceId sourceId = SourceId::none(),
        MessageContext ctx = {},
        std::vector<Log> logs = {}
    );

    /** @brief Move constructor. Transfers ownership of the scoped task. */
    MessageTaskScoped(MessageTaskScoped&& other) noexcept;

    /**
     * @brief Destructor.
     * If the task hasn't been finalized (completed/aborted), it sends a FAIL message.
     */
    ~MessageTaskScoped();

    // Non-copyable to prevent duplicate "Finish" messages.
    MessageTaskScoped(const MessageTaskScoped&) = delete;


    //-----------------------------------------------------------------------------
    // Logical operators
    //-----------------------------------------------------------------------------

    // Move assignment.
    MessageTaskScoped& operator=(MessageTaskScoped&& other) noexcept;

    // Non-copyable to prevent duplicate "Finish" messages.
    MessageTaskScoped& operator=(const MessageTaskScoped&) = delete;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------
    
    /** @brief Transition the task to STARTED state. */
    void start(std::vector<Log> logs = {});

    /** @overload */
    void start(Log logs);


    /** @brief Transition the task to SUCCESS state and finalize. */
    void complete(std::vector<Log> logs = {});

    /** @overload */
    void complete(Log logs);


    /** @brief Transition the task to FAIL state and finalize. */
    void abort(std::vector<Log> logs);

    /** @overload */
    void abort(Log logs);

    /**
     * @brief Utility to abort if a condition is met.
     * @param condition If true, abort() is called.
     * @param logs Logs to attach if aborted.
     * @return The value of the condition.
     */
    [[nodiscard]] bool abortIf(bool condition, std::vector<Log> logs);

    /** @overload */
    [[nodiscard]] bool abortIf(bool condition, Log logs);

private:
    std::shared_ptr<MessageDispatcher> messageBus_;

    const IMessageTag* messageTag_;

    MessageTask::TaskStatus status_;

    SourceId sourceId_;

    MessageContext ctx_;

    // Prevents the destructor from sending redundant FAIL messages.
    bool finalized_{ false };
};
