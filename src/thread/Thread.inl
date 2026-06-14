#pragma once
#include "Thread.hpp"


template<typename Func, typename Class, typename ...Args>
inline void Thread::enqueueTask(
    Func&& func,
    Class* obj,
    Args&& ...args)
{
    auto argsTuple = std::make_tuple(std::forward<Args>(args)...);

    enqueueTask(
        [func = std::forward<Func>(func),
        obj,
        argsTuple = std::move(argsTuple)]() mutable
        {
            std::apply([&](auto&&... unpackedArgs) {
                std::invoke(
                    std::move(func),
                    obj,  // Object pointer for member function
                    std::forward<decltype(unpackedArgs)>(unpackedArgs)...  // Args
                );

                }, std::move(argsTuple));
        });
}
