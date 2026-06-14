#ifdef __APPLE__
#pragma once
#include "../os_adapter/CameraOSAdapter.hpp"


/**
 * @brief Apple-specific adapter using the AVFoundation API.
 */
class AVFoundation : public CameraOSAdapter<AVFoundation>
{
public:
    static std::vector<std::string> listDeviceIds();
};
#endif