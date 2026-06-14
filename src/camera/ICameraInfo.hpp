#pragma once
#include <parameter/ParameterOwnerInfo.hpp>
#include "ICameraDefs.hpp"
#include "CameraState.hpp"


/**
 * @brief Abstract base class representing a generic read-only interface for a Camera.
 */
class ICameraInfo : public virtual ParameterOwnerInfo
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    ICameraInfo() = default;

    virtual ~ICameraInfo() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    virtual bool isDetected() const = 0;

    virtual bool isConnected() const = 0;

    virtual bool isGrabbing() const = 0;

    virtual bool disconnectionRequested() const = 0;

    virtual CameraState state() const = 0;

    virtual bool areParametersPersistent() const = 0;

    virtual const std::string& interfaceId() const = 0;

    virtual const std::string& serialNumber() const = 0;

    virtual CameraInterface interfaceType() const = 0;
};
