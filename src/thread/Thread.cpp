#include "Thread.hpp"


Thread::~Thread()
{
    shutdown();
}

///////////////////////////////////////////////////////////

Thread::StartResult Thread::start()
{
    if (isOn_.load())
        return StartResult::ALREADY_RUNNING;

    stopWorker_.store(false);

    workerThread_ = std::thread(&Thread::workerThreadLoop, this);

    threadId_ = workerThread_.get_id();

    isOn_.store(true);

    return StartResult::STARTED;
}

Thread::ShutdownResult Thread::shutdown()
{
    if (!isOn_.load())
        return ShutdownResult::ALREADY_STOPPED; // already stopped

    {
        std::lock_guard<std::mutex> lock(queueMutex_);
        stopWorker_.store(true);
        queueCV_.notify_one();
    }

    ShutdownResult ret{ ShutdownResult::STOPPED };

    if (std::this_thread::get_id() == workerThread_.get_id())
    {
        // can't join self
        workerThread_.detach();

        ret = ShutdownResult::DETACHED_SELF;
    }
    else if (workerThread_.joinable())
    {
        workerThread_.join();

        ret = ShutdownResult::STOPPED;
    }

    isOn_.store(false);

    return ret;
}

size_t Thread::getTaskQueueSize() const
{
    std::lock_guard<std::mutex> lock(queueMutex_);
    return taskQueue_.size();
}

std::thread::id Thread::getThreadId() const
{
    return isOn_.load() ? threadId_ : std::thread::id{};
}

bool Thread::isDoingATask() const
{
    return doingTask_.load();
}

void Thread::workerThreadLoop()
{
    while (true)
    {
        MoveOnlyFunction<void()> task;
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            queueCV_.wait(lock, [this] { return !taskQueue_.empty() || stopWorker_.load(); });

            if (stopWorker_.load() && taskQueue_.empty()) break;

            task = std::move(taskQueue_.front());
            taskQueue_.pop_front();
        }

        try
        {
            doingTask_.store(true);
            if (task) {
                task();
            }
        }
        catch (...)
        {
            // log
        }
        doingTask_.store(false);
    }
}

void Thread::enqueueTask(MoveOnlyFunction<void()> task)
{
    if (stopWorker_.load())
        return; // reject new tasks after shutdown

    std::lock_guard<std::mutex> lock(queueMutex_);

    taskQueue_.push_back(std::move(task));       // FIFO
    queueCV_.notify_one();
}