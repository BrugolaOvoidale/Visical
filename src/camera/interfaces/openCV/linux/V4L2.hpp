#ifdef __linux__
#pragma once
#include "../os_adapter/CameraOSAdapter.hpp"


/**
 * @brief Linux-specific adapter using Video4Linux2.
 * 
 * Enumerates devices by scanning the '/dev/video*' nodes in the filesystem.
 */
class V4L2 : public CameraOSAdapter<V4L2>
{
public:
	static std::vector<std::string> listDeviceIds();
};
#endif