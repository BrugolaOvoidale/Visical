#pragma once
#include "AsyncWorker.hpp"


template<typename Class, typename... MethodArgs, typename... CallArgs>
inline void AsyncWorker::enqueueMessageTask(
    const MessageTag<MessageTask>& msgTag,
    SourceId sourceId,
    MessageContext msgCtx,
    TaskResult(Class::*method)(MethodArgs...),
    Class* obj,
    CallArgs&& ...args)
{
    MessageTaskScoped msg(msgTag, msgDispatcher_, sourceId, std::move(msgCtx));

     auto argsTuple = std::make_tuple(std::forward<CallArgs>(args)...);

    worker_.enqueueTask(
        [msg = std::move(msg),
        method,
        obj,
        argsTuple = std::move(argsTuple)]() mutable
        {
            msg.start();
            try
            {
                std::apply([&](auto&&... unpackedArgs)
                    {
                        TaskResult taskRes = (obj->*method)(
                            std::forward<decltype(unpackedArgs)>(unpackedArgs)...
                            );

                        if (taskRes.isSuccess())
                            msg.complete(taskRes.takeLogs());
                        else
                            msg.abort(taskRes.takeLogs());

                    }, std::move(argsTuple));
            }
            catch (const std::exception& e)
            {
                msg.abort({ e.what(), Log::Level::LVL_ERROR });
            }
            catch (...)
            {
                msg.abort({ "Unknown exception", Log::Level::LVL_ERROR});
            }
        });
}

template<typename Payload, typename Class,  typename... MethodArgs, typename... CallArgs>
inline void AsyncWorker::enqueueMessageTask(
    const MessageTag<MessageTaskP<Payload>>& msgTag,
    SourceId sourceId,
    MessageContext msgCtx,
    TaskResultP<Payload>(Class::*method)(MethodArgs...),
    Class* obj,
    CallArgs&& ...args)
{
    MessageTaskScopedP msg(msgTag, msgDispatcher_, sourceId, std::move(msgCtx));

     auto argsTuple = std::make_tuple(std::forward<CallArgs>(args)...);

    worker_.enqueueTask(
        [msg = std::move(msg),
        method,
        obj,
        argsTuple = std::move(argsTuple)]() mutable
        {
            msg.start();
            try
            {
                std::apply([&](auto&&... unpackedArgs) {
                    TaskResultP<Payload> taskRes = (obj->*method)(
                        std::forward<decltype(unpackedArgs)>(unpackedArgs)...
                        );

                    if (taskRes.isSuccess())
                        msg.complete(taskRes.takePayload(), taskRes.takeLogs());
                    else
                        msg.abort(taskRes.takeLogs());

                    }, std::move(argsTuple));
            }
            catch (const std::exception& e)
            {
                msg.abort({ e.what(), Log::Level::LVL_ERROR });
            }
            catch (...)
            {
                msg.abort({ "Unknown exception", Log::Level::LVL_ERROR });
            }
        });
}
