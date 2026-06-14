#include "AsyncWorker.hpp"


AsyncWorker::AsyncWorker()
{
    worker_.start();
}

void AsyncWorker::shutdown()
{
    worker_.shutdown();
}