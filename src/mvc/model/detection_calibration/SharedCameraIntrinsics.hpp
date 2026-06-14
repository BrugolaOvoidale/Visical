#pragma once
#include <memory>
#include <atomic>
#include <calibration/CameraIntrinsics.hpp>
#include <message_bus/MessageDispatcherProxy.hpp>


/**
 * @brief Thread-safe container for exchanging camera calibration data between threads.
 *
 * This class provides an atomic wrapper around a std::shared_ptr to CameraIntrinsics.
 * It derives from MessageDispatcherProxy, and it will publish a MSG_CAM_INTRINSICS_CHANGED
 * message at each SharedCameraIntrinsics::set() call.
 */
class SharedCameraIntrinsics : public MessageDispatcherProxy
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    /**
     * @brief Constructs the container with an initial set of intrinsics.
     * @param initial The starting calibration data to be stored.
     */
    explicit SharedCameraIntrinsics(CameraIntrinsics initial);

    ~SharedCameraIntrinsics() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /**
     * @brief Atomically retrieves a shared pointer to the current intrinsics.
     * 
     * This operation is non-blocking. The returned pointer ensures
     * the data remains valid for the caller even if set() is called by another
     * thread immediately after.
     * 
     * @return A shared_ptr to the current CameraIntrinsics.
     */
    std::shared_ptr<CameraIntrinsics> get() const;

    /**
     * @brief Atomically replaces the current intrinsics with a new set.
     * 
     * It will publish a MSG_CAM_INTRINSICS_CHANGED message at each call.
     * @param next The new calibration data to promote as the current version.
     */
    void set(CameraIntrinsics next);

private:
    // The atomic shared pointer storage
    std::atomic<std::shared_ptr<CameraIntrinsics>> current_;
};
