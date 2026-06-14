#include "CameraManager.hpp"
#include <UtilityFunctions.hpp>
#include "ICameraProxy.hpp"
#include "CameraManagerMessages.hpp"


CameraManager::~CameraManager()
{
    unregisterAllCameras();

}

///////////////////////////////////////////////////////////////////////

void CameraManager::bindCameraMessages(const std::shared_ptr<ICamera>& camera)
{
    if (!camera)
        return;

    camera->subscribe(MSG_SHUTDOWN_CAMERA, &CameraManager::onCameraShutdown, this);
}

void CameraManager::onCameraShutdown(const MessageTask& msg)
{
    switch (msg.status())
    {
        case MessageTask::TaskStatus::PENDING:
        case MessageTask::TaskStatus::STARTED:
            break;

        case MessageTask::TaskStatus::REJECTED:
        case MessageTask::TaskStatus::SUCCESS:
        case MessageTask::TaskStatus::FAIL:
        {
            const std::string camId = msg.context().getAttribute<std::string>(CAM_SERIAL);

            TaskResult unregRes = doUnregisterCamera(camId);
            std::vector<Log> exitLogs = msg.getLogs();
            UtilityFunctions::moveInto(unregRes.takeLogs(), exitLogs);

            publish(Message(MSG_CAM_MGR_UNREGISTER_CAM, SourceId::none(), { { CAM_SERIAL, camId } }, std::move(exitLogs)));

            break;
        }
    }
}

///////////////////////////////////////////////////////////////////////

std::shared_ptr<ICameraProxy> CameraManager::getCameraProxy(const std::string& id)
{
    std::shared_lock lock(cameraProxiesMutex_);
    auto it = cameraProxies_.find(id);
    return it != cameraProxies_.end() ? it->second : nullptr;
}

std::vector<std::shared_ptr<ICameraProxy>> CameraManager::getAllCameraProxies()
{
    std::shared_lock lock(cameraProxiesMutex_);

    std::vector<std::shared_ptr<ICameraProxy>> camProxies;
    camProxies.reserve(cameraProxies_.size());

    for (const auto& [_, cam] : cameraProxies_)
    {
        camProxies.push_back(cam);
    }

    return camProxies;
}

bool CameraManager::registerCamera(const std::shared_ptr<ICamera>& camera)
{
    if (!camera)
        return false;


    const std::string& serial = camera->serialNumber();

    bool success;
    {
        std::unique_lock lock(registeredCamerasMutex_);

        auto it = registeredCameras_.find(serial);
        if (it != registeredCameras_.end())
            return true;

        success = registeredCameras_.emplace(serial, camera).second;
    }

    if (success)
    {
        std::unique_lock lock(cameraProxiesMutex_);

        auto it = cameraProxies_.find(serial);
        if (it != cameraProxies_.end())
            return true;

        success = cameraProxies_.emplace(serial, 
            std::make_shared<ICameraProxy>(camera)
        ).second;
    }

    if (success)
    {
        bindCameraMessages(camera);
        
        publish(Message(MSG_CAM_MGR_REGISTER_CAM, SourceId::none(), { { CAM_SERIAL, serial } }));
    }

    return success;
}

TaskResult CameraManager::unregisterCamera(const std::string& id)
{
    std::shared_ptr<ICamera> camera = getCamera(id);

    if (!camera)
    {
        return { false, { "Camera not found", Log::Level::LVL_ERROR } };
	}

    return camera->shutdown();
}

void CameraManager::unregisterAllCameras()
{
    std::vector<std::string> ids;
    {
        // Take a snapshot of the IDs under lock
        std::shared_lock lock(registeredCamerasMutex_);
        ids.reserve(registeredCameras_.size());
        for (const auto& [id, _] : registeredCameras_)
        {
            ids.push_back(id);
        }
    }

    // Loop through and unregister each one
    for (const auto& id : ids)
    {
        unregisterCamera(id);
    }
}

size_t CameraManager::getRegisteredCameraCount()
{
    std::shared_lock lock(registeredCamerasMutex_);
    return registeredCameras_.size();
}

void CameraManager::disconnectAllCameras()
{
    std::shared_lock lock(registeredCamerasMutex_);

    for (const auto& [_, cam] : registeredCameras_)
    {
        cam->disconnect();
    }
}

///////////////////////////////////////////////////////////////////////

 std::shared_ptr<ICamera> CameraManager::getCamera(const std::string& id)
{
    std::shared_lock lock(registeredCamerasMutex_);
    auto it = registeredCameras_.find(id);
    return it != registeredCameras_.end() ? it->second : nullptr;
}

TaskResult CameraManager::doUnregisterCamera(const std::string& id)
{
    {
        std::unique_lock lock(registeredCamerasMutex_);

        registeredCameras_.erase(id);
    }

    {
        std::unique_lock lock(cameraProxiesMutex_);

        cameraProxies_.erase(id);
    }

    return true;
}
