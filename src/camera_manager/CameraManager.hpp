#pragma once
#include "CameraManagerMessages.hpp"
#include <message_bus/MessageDispatcherProxy.hpp>
#include <task_result/TaskResult.hpp>


// Forward declarations
class ICamera;
class ICameraProxy;


/**
 * @brief Manages the lifecycle of camera devices.
 * 
 * This acts as a mediator between physical camera implementations (ICamera)
 * and their public-facing interfaces (ICameraProxy), thus keeping it the
 * only responsible for their lifecycle.
 */
class CameraManager : public MessageDispatcherProxy
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    CameraManager() = default;

    ~CameraManager();


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /**
     * @brief Retrieves a proxy interface for a specific camera.
     * @param id The unique identifier of the camera.
     * @return A shared pointer to the proxy, or nullptr if not found.
     */
    std::shared_ptr<ICameraProxy> getCameraProxy(const std::string& id);

    /** @brief Returns a list of all currently available camera proxies. */
    std::vector<std::shared_ptr<ICameraProxy>> getAllCameraProxies();

    /**
     * @brief Registers a new camera implementation and creates its associated proxy.
     * @param camera The camera instance to manage.
     * @return false if camera is nullptr, otherwise true.
     */
    bool registerCamera(const std::shared_ptr<ICamera>& camera);

    /**
     * @brief Unregisters a camera by its Id.
     * @param id The unique identifier of the camera.
     * @return false if there is no camera with said id, true otherwise. Check ICamera::shutdown
     * for more details.
     */
    TaskResult unregisterCamera(const std::string& id);

    /** @brief Unregisters and cleans up all managed cameras. */
    void unregisterAllCameras();

    /** @brief Returns the total number of registered cameras. */
    size_t getRegisteredCameraCount();

    /** @brief Triggers a hardware-level disconnection for all registered cameras. */
    void disconnectAllCameras();

private:
    // Binds camera-specific events to the internal message bus.
    void bindCameraMessages(const std::shared_ptr<ICamera>& camera);

    // Internal callback for camera shutdown messages.
    void onCameraShutdown(const MessageTask& msg);

    // Core logic for camera unregistration.
    TaskResult doUnregisterCamera(const std::string& id);

    // Internal helper to find an ICamera instance by Id
    std::shared_ptr<ICamera> getCamera(const std::string& id);

private:
    // Protects registeredCameras_ map.
    mutable std::shared_mutex registeredCamerasMutex_;
    std::unordered_map<std::string, std::shared_ptr<ICamera>> registeredCameras_;

    // Protects cameraProxies_ map.
    mutable std::shared_mutex cameraProxiesMutex_;
    std::unordered_map<std::string, std::shared_ptr<ICameraProxy>> cameraProxies_;
};
