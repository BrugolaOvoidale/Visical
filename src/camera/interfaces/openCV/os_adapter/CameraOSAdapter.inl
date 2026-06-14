#pragma once
#include "CameraOSAdapter.hpp"


template<typename Derived>
std::vector<std::string> CameraOSAdapter<Derived>::listDeviceIds()
{
	return Derived::listDeviceIds();
}