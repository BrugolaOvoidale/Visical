#include "ICameraProxy.hpp"
#include <utility/MoveOnlyFunction.hpp>

ICameraProxy::ICameraProxy(const std::shared_ptr<ICamera>& iCam)
    : iCam_(iCam)
{
    if (!iCam)
    {
        throw std::runtime_error("iCam is nullptr");
    }
}

///////////////////////////////////////////////////////////////////////////////////////

TaskEnqueueResult ICameraProxy::check()
{
    return iCam_->check();
}

TaskEnqueueResult ICameraProxy::connect()
{
    return iCam_->connect();
}

TaskEnqueueResult ICameraProxy::disconnect()
{
    return iCam_->disconnect();
}

TaskEnqueueResult ICameraProxy::startGrab(int timeout)
{
    return iCam_->startGrab(timeout);
}

TaskEnqueueResult ICameraProxy::stopGrab()
{
    return iCam_->stopGrab();
}

TaskEnqueueResult ICameraProxy::takeSnapshot(
    MoveOnlyFunction<void(const TaskResultP<std::shared_ptr<const CvImage>>&)> func,
    int timeout)
{
    return iCam_->takeSnapshot(std::move(func), timeout);
}

std::shared_ptr<const CvImage> ICameraProxy::getLatestFrame()
{
    return iCam_->getLatestFrame();
}

TaskResult ICameraProxy::setParameter(
    const std::string& paramId,
    const std::string& categoryId,
    const std::string& value)
{
    return iCam_->setParameter(paramId, categoryId, value);
}

TaskResult ICameraProxy::setParameter(
    const std::string& paramId,
    const std::string& categoryId,
    int value)
{
    return iCam_->setParameter(paramId, categoryId, value);
}

TaskResult ICameraProxy::setParameter(
    const std::string& paramId,
    const std::string& categoryId,
    double value)
{
    return iCam_->setParameter(paramId, categoryId, value);
}

TaskResult ICameraProxy::setParameter(
    const std::string& paramId,
    const std::string& categoryId,
    bool value)
{
    return iCam_->setParameter(paramId, categoryId, value);
}

TaskResult ICameraProxy::executeCommand(
    const std::string& paramId,
    const std::string& categoryId)
{
    return iCam_->executeCommand(paramId, categoryId);
}

TaskResult ICameraProxy::resetParameter(
    const std::string& paramId,
    const std::string& categoryId)
{
    return iCam_->resetParameter(paramId, categoryId);
}

std::shared_ptr<ICameraInfo> ICameraProxy::getCameraView() const
{
    return iCam_->getCameraView();
}

std::shared_ptr<ParameterInfo> ICameraProxy::getParameter(
    const std::string& paramId,
    const std::string& categoryId) const
{
    return iCam_->getParameter(paramId, categoryId);
}

std::vector<std::shared_ptr<ParameterInfo>> ICameraProxy::getFilteredParams(
    const std::optional<std::string>& category,
    std::optional<VisibilityLevel> visibility,
    bool inAlphabeticalOrder) const
{
    return iCam_->getFilteredParams(category, visibility, inAlphabeticalOrder);
}

std::shared_ptr<ParameterInfo> ICameraProxy::getDefaultParameter(
    const std::string& paramId,
    const std::string& categoryId) const
{
    return iCam_->getDefaultParameter(paramId, categoryId);
}

std::vector<std::shared_ptr<ParameterInfo>> ICameraProxy::getDefaultFilteredParams(
    const std::optional<std::string>& category,
    std::optional<VisibilityLevel> visibility,
    bool inAlphabeticalOrder) const
{
    return iCam_->getDefaultFilteredParams(category, visibility, inAlphabeticalOrder);
}

bool ICameraProxy::isDetected() const
{
    return iCam_->isDetected();
}

bool ICameraProxy::isConnected() const
{
    return iCam_->isConnected();
}

bool ICameraProxy::isGrabbing() const
{
    return iCam_->isGrabbing();
}

bool ICameraProxy::disconnectionRequested() const
{
    return iCam_->disconnectionRequested();
}

CameraState ICameraProxy::state() const
{
    return iCam_->state();
}

void ICameraProxy::setParametersPersistence(bool arePersistent)
{
    iCam_->setParametersPersistence(arePersistent);
}

bool ICameraProxy::areParametersPersistent() const
{
    return iCam_->areParametersPersistent();
}

const std::string& ICameraProxy::interfaceId() const
{
    return iCam_->interfaceId();
}

const std::string& ICameraProxy::serialNumber() const
{
    return iCam_->serialNumber();
}

CameraInterface ICameraProxy::interfaceType() const
{
    return iCam_->interfaceType();
}

TaskResultP<AcquisitionMode> ICameraProxy::acquisitionMode() const
{
	return iCam_->acquisitionMode();
}