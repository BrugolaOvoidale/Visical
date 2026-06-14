#pragma once
#include "ICamera.hpp"


template<typename Func, typename Class>
inline TaskEnqueueResult ICamera::takeSnapshot(
    Func&& func,
    Class* obj,
    int timeout)
{
    auto wrapper =
        [obj,
        f = std::forward<Func>(func)]
        (const TaskResultP<std::shared_ptr<const CvImage>>& frame) mutable
        {
            (obj->*f)(frame);
        };

    return takeSnapshot(std::move(wrapper), timeout);
}
