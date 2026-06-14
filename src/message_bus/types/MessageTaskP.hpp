#pragma once
#include "MessageTask.hpp"


/**
 * @brief A message representing the state and logs of a discrete task
 * that produce a valid payload when it completes successfully.
 *
 * This class inherits from MessageTask and adds a templatable
 * payload, wrapped in a 'std::optional', that has value ONLY if
 * TaskStatus::SUCCESS.
 */
template<typename T>
class MessageTaskP : public MessageTask
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    /**
     * @brief Detailed constructor for MessageTask.
     * 
     * Note that initial status is always PENDING when using public ctor,
     * as we can better handle the payload when is SUCCESS.
     * 
     * @param tag The message identifier tag.
     * @param id The source identifier.
     * @param ctx Execution context/metadata.
     * @param logs Initial collection of log entries.
     */
    MessageTaskP(
        const IMessageTag& tag,
        SourceId sourceId = SourceId::none(),
        MessageContext ctx = {},
        std::vector<Log> logs = {}
    );

    std::shared_ptr<Message> clone() const override;

    ~MessageTaskP() = default;


    //-----------------------------------------------------------------------------
    // Construction from factories
    //-----------------------------------------------------------------------------

    /** @brief Creates a REJECTED status message. */
    static MessageTaskP rejected(
        const IMessageTag& tag,
        std::vector<Log> logs,
        SourceId sourceId = SourceId::none(),
        MessageContext ctx = {}
    );
    static MessageTaskP rejected(
        const IMessageTag& tag,
        Log logs,
        SourceId sourceId,
        MessageContext ctx
    );


    /** @brief Creates a PENDING status message. */
    static MessageTaskP pending(
        const IMessageTag& tag,
        SourceId sourceId = SourceId::none(),
        MessageContext ctx = {},
        std::vector<Log> logs = {}
    );
    static MessageTaskP pending(
        const IMessageTag& tag,
        SourceId sourceId,
        MessageContext ctx,
        Log logs
    );


    /** @brief Creates a STARTED status message. */
    static MessageTaskP started(
        const IMessageTag& tag,
        SourceId sourceId = SourceId::none(),
        MessageContext ctx = {},
        std::vector<Log> logs = {}
    );
    static MessageTaskP started(
        const IMessageTag& tag,
        SourceId sourceId,
        MessageContext ctx,
        Log logs
    );


    /** @brief Creates a SUCCESS status message. Valid payload is an hard requirement */
    static MessageTaskP success(
        const IMessageTag& tag,
        T payload,
        SourceId sourceId = SourceId::none(),
        MessageContext ctx = {},
        std::vector<Log> logs = {}
    );
    static MessageTaskP success(
        const IMessageTag& tag,
        T payload,
        SourceId sourceId,
        MessageContext ctx,
        Log logs
    );


    /** @brief Creates a FAIL status message. */
    static MessageTaskP fail(
        const IMessageTag& tag,
        std::vector<Log> logs,
        SourceId sourceId = SourceId::none(),
        MessageContext ctx = {}
    );
    static MessageTaskP fail(
        const IMessageTag& tag,
        Log logs,
        SourceId sourceId,
        MessageContext ctx
    );


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /**
     * @brief Safe payload access. Has value only if SUCCESS.
     */
    const std::optional<T>& tryGetPayload() const;

    /**
     * @brief Unsafe payload access, will call 'std::optional::value()' without checking. Has value only if SUCCESS.
     */
    const T& getPayload() const;

protected:
    // Protected constructor for factories methods to specify the TaskStatus.
    MessageTaskP(
        const IMessageTag& tag,
        SourceId sourceId,
        MessageContext ctx,
        MessageTask::TaskStatus s,
        std::optional<T> p,
        std::vector<Log> logs
    );

private:
    std::optional<T> payload_;
};

////////////////////////////////////////////////////////////////////

/**
 * @brief A RAII wrapper for task reporting.
 *
 * This class automatically manages the lifecycle of a MessageTask.
 * - Upon construction, it sends a **PENDING** message.
 * - Calling start() sends a **STARTED** message.
 * - Calling complete() sends a **SUCCESS** message paired with the payload.
 * - **Crucially**: If the object is destroyed before complete() is called,
 * it automatically sends a **FAIL** message in the destructor.
 */
template<typename T>
class MessageTaskScopedP
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    /**
     * @brief Constructor. Immediately broadcasts a PENDING message via the bus.
     */
    MessageTaskScopedP(
        const IMessageTag& messageTag,
        const std::shared_ptr<MessageDispatcher>& messageBus,
        SourceId sourceId = SourceId::none(),
        MessageContext ctx = {},
        std::vector<Log> logs = {}
    );

    /** @brief Move constructor. Transfers ownership of the scoped task. */
    MessageTaskScopedP(MessageTaskScopedP&& other) noexcept;

    /**
     * @brief Destructor.
     * If the task hasn't been finalized (completed/aborted), it sends a FAIL message.
     */
    ~MessageTaskScopedP();

    // Non-copyable to prevent duplicate "Finish" messages.
    MessageTaskScopedP(const MessageTaskScopedP&) = delete;


    //-----------------------------------------------------------------------------
    // Logical operators
    //-----------------------------------------------------------------------------
    
    // Move assignment.
    MessageTaskScopedP& operator=(MessageTaskScopedP&& other) noexcept;

    // Non-copyable to prevent duplicate "Finish" messages.
    MessageTaskScopedP& operator=(const MessageTaskScopedP&) = delete;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------
    
    /** @brief Transition the task to STARTED state. */
    void start(std::vector<Log> logs = {});

    /** @overload */
    void start(Log logs);


    /** @brief Transition the task to SUCCESS state and finalize. */
    void complete(T payload, std::vector<Log> logs = {});

    /** @overload */
    void complete(T payload, Log logs);


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
    bool finalized_ = false;
};


#include "MessageTaskP.inl"


template<typename T>
MessageTaskScopedP(
    const MessageTag<MessageTaskP<T>>&,
    const std::shared_ptr<MessageDispatcher>&,
    SourceId,
    MessageContext,
    std::vector<Log> = {})
    -> MessageTaskScopedP<T>;

//template<typename T>
//MessageTaskScopedP(
//    const IMessageTag&,
//    const std::shared_ptr<MessageDispatcher>&,
//    SourceId,
//    MessageContext,
//    std::vector<Log> = {})
//    -> MessageTaskScopedP<T>;
