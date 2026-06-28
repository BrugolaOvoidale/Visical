#include "SharedCameraIntrinsics.hpp"
#include "SharedCameraIntrinsicsMessages.hpp"

SharedCameraIntrinsics::SharedCameraIntrinsics(CameraIntrinsics initial)
    : current_(std::make_shared<CameraIntrinsics>(std::move(initial)))
{
    currentMutex_ = new std::recursive_mutex();
}

std::shared_ptr<CameraIntrinsics> SharedCameraIntrinsics::get() const
{
    std::lock_guard<std::recursive_mutex> lock(*currentMutex_);
    return current_;
}

void SharedCameraIntrinsics::set(CameraIntrinsics next)
{
    {
        std::lock_guard<std::recursive_mutex> lock(*currentMutex_);
        current_ = std::make_shared<CameraIntrinsics>(std::move(next));
    } 
    publish(Message(MSG_CAM_INTRINSICS_CHANGED));
}