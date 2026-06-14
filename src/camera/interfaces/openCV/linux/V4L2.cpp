#ifdef __linux__
#include "V4L2.hpp"
#include <filesystem>


std::vector<std::string> V4L2::listDeviceIds()
{
    std::vector<std::string> ids;
    std::string path = "/dev/v4l/by-id";

    if (std::filesystem::exists(path))
    {
        for (const auto& entry : std::filesystem::directory_iterator(path))
        {
            ids.push_back(entry.path().string());
        }
    }
    return ids;
}
#endif