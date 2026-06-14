#include "SharedCameraIntrinsics.hpp"
#include "SharedCameraIntrinsicsMessages.hpp"


SharedCameraIntrinsics::SharedCameraIntrinsics(CameraIntrinsics initial)
    : current_(std::make_shared<CameraIntrinsics>(std::move(initial)))
{
}
///////////////////////////////////////////////////////////////////////

std::shared_ptr<CameraIntrinsics> SharedCameraIntrinsics::get() const
{
    return current_.load();
}

void SharedCameraIntrinsics::set(CameraIntrinsics next)
{
    current_.store(
        std::make_shared<CameraIntrinsics>(std::move(next))
    );

    publish(Message(MSG_CAM_INTRINSICS_CHANGED));
}
