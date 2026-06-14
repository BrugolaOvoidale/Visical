#pragma once
#include <functional>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <deque>
#include <memory>
#include <utility>


/**
 * @brief A worker thread with an asynchronous task queue.
 *
 * This class encapsulates a single 'std::thread' and a thread-safe task queue.
 * It allows for offloading functional tasks and class methods to be executed
 * sequentially in the background.
 */
class Thread
{
public:
    //-----------------------------------------------------------------------------
    // Enums
    //-----------------------------------------------------------------------------

    enum class StartResult {
        STARTED,            ///< Thread was successfully launched.
        ALREADY_RUNNING,    ///< Thread is already active.
    };

    enum class ShutdownResult {
        STOPPED,            ///< Thread joined and finished successfully.
        ALREADY_STOPPED,    ///< Thread was not running.
        DETACHED_SELF       ///< Shutdown was called from within the worker itself.
    };


    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    /** @brief Initializes the thread object without starting the worker loop. */
    Thread() = default;

    /**
     * @brief Destructor. Automatically calls shutdown() to ensure the worker thread is joined.
     */
    ~Thread();


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /**
     * @brief Launches the background worker thread.
     * @return StartResult indicating success or current state.
     */
    StartResult start();

    /**
     * @brief Signals the worker thread to stop and waits for it to join.
     * All remaining tasks will be drained befor the thread is joined.
     * @return ShutdownResult indicating how the thread was terminated.
     */
    ShutdownResult shutdown();

    /** @return The number of tasks currently waiting in the queue. */
    size_t getTaskQueueSize() const;

    /** @return The unique identifier of the internal worker thread. */
    std::thread::id getThreadId() const;

    /** @return True if the worker is currently executing a task. */
    bool isDoingATask() const;

    /**
     * @brief Enqueues a standalone function or lambda for execution.
     * @param task A move-only function to be executed.
     */
    void enqueueTask(std::move_only_function<void()> task);

    /**
     * @brief Enqueues a member function of a specific class instance.
     * @tparam Func The member function pointer type.
     * @tparam Class The class type owning the function.
     * @tparam Args Argument types for the function.
     * @param func The member function to call (e.g., &MyClass::myMethod).
     * @param obj Pointer to the instance to call the method on.
     * @param args Arguments to pass to the method.
     */
    template<typename Func, typename Class, typename ...Args>
    void enqueueTask(
        Func&& func,
        Class* obj,
        Args&& ...args
    );

private:
    // The main loop executed by the background thread.
    void workerThreadLoop();

private:
    mutable std::mutex threadMutex_;

    // The underlying system thread.
    std::thread workerThread_;

    // Flag to signal the thread to exit.
    std::atomic<bool> stopWorker_{ false };

    // Cached thread Id for quick access
	std::thread::id threadId_;

    // Queue of pending work.
    std::deque<std::move_only_function<void()>> taskQueue_;

    mutable std::mutex queueMutex_;
    
    // CV to wake the worker when tasks arrive.
    std::condition_variable queueCV_;

    // Set to true while a task is in progress.
    std::atomic<bool> doingTask_{ false };

    // Tracks if start() has been called.
    std::atomic<bool> isOn_{ false };
};

#include "Thread.inl"
