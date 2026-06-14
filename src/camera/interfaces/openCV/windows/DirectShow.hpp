#ifdef _WIN32
#pragma once
#include "../os_adapter/CameraOSAdapter.hpp"
#pragma comment(lib, "strmiids.lib")


/**
 * @brief Windows-specific adapter using the DirectShow API.
 */
class DirectShow : public CameraOSAdapter<DirectShow>
{
public:
	static std::vector<std::string> listDeviceIds();
};
#endif