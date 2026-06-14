#pragma once
#include <message_bus/MessageDispatcherProxy.hpp>
#include <task_result/TaskResult.hpp>
#include <thread/Thread.hpp>


/**
 * @brief Base class for objects that process tasks and messages on a dedicated background thread.
 * 
 * This class encapsulates a background Thread and a MessageDispatcher.
 * It allows derived classes to subscribe to messages and enqueue tasks apired with a message that
 * will be executed asynchronously.
 */
class AsyncWorker : public MessageDispatcherProxy
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    /** @brief Initializes and start the thread object. */
    AsyncWorker();

    ~AsyncWorker() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /**
     * @brief Gracefully stops and clear the internal resources.
     */
    void shutdown();

protected:
    /**
     * @brief Enqueues a task associated with a standard MessageTask.
     * 
     * @param msgTag Tag identifying the task message.
     * @param sourceId Identifier of the task source.
     * @param msgCtx Contextual information for the message.
     * @param method The member function representing the task logic.
     * @param obj The instance on which to run the task.
     * @param args Arguments to pass to the method.
     */
    template<typename Class, typename... MethodArgs, typename... CallArgs>
    void enqueueMessageTask(
        const MessageTag<MessageTask>& msgTag,
        SourceId sourceId,
        MessageContext msgCtx,
        TaskResult(Class::* method)(MethodArgs...),
        Class* obj,
        CallArgs&&... args
    );

    /**
     * @brief Enqueues a task associated with a MessageTaskP (Payload-based).
     * 
     * @tparam Payload The type of data carried by the task.
     * @param msgTag Tag identifying the task message.
     * @param sourceId Identifier of the task source.
     * @param msgCtx Contextual information for the message.
     * @param method The member function representing the task logic, returning TaskResultP.
     * @param obj The instance on which to run the task.
     * @param args Arguments to pass to the method.
     */
    template<typename Payload, typename Class, typename... MethodArgs, typename... CallArgs>
    void enqueueMessageTask(
        const MessageTag<MessageTaskP<Payload>>& msgTag,
        SourceId sourceId,
        MessageContext msgCtx,
        TaskResultP<Payload>(Class::* method)(MethodArgs...),
        Class* obj,
        CallArgs&&... args
    );

protected:
    // Dedicated background thread for this worker.
    Thread worker_;
};

#include "AsyncWorker.inl"