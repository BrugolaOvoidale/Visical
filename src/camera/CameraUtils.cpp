#include "CameraUtils.hpp"
#ifdef WITH_GENICAM
#include "interfaces/aravis/AravisGenICamCamera.hpp"
#endif
#include "interfaces/openCV/OpenCVCamera.hpp"


#ifdef WITH_GENICAM
std::shared_ptr<ICamera> CameraUtils::createAravisCam(const std::string& camSerial)
{
    auto devices = enumerateAravis();

    for (const auto& pair : devices)
    {
        if (pair.serial == camSerial)
        {
            return AravisGenICamCamera::create(camSerial, pair.ifaceId);
        }
    }

    return nullptr;
}

std::vector<CameraUtils::Descriptor> CameraUtils::enumerateAravis()
{
    std::vector<CameraUtils::Descriptor> devices;

    arv_update_device_list();
    unsigned int num_devices = arv_get_n_devices();

    for (unsigned int i = 0; i < num_devices; i++)
    {
        const char* serial = arv_get_device_serial_nbr(i);
        const char* id = arv_get_device_id(i);
        if (serial && id)
        {
            devices.emplace_back(serial, id);
        }
    }

    return devices;
}
#endif

std::shared_ptr<ICamera> CameraUtils::createOpenCVCamera(const std::string& camSerial)
{
    auto devices = enumerateOpenCV();

    for (const auto& pair : devices)
    {
        if (pair.serial == camSerial)
        {
            return OpenCVCamera::create(camSerial, pair.ifaceId);
        }
    }

    return nullptr;
}

std::vector<CameraUtils::Descriptor> CameraUtils::enumerateOpenCV()
{
    std::vector<CameraUtils::Descriptor> devices;

    std::vector<std::string> list = OpenCVCamera::OSInterface::listDeviceIds();

    for (const auto& l : list)
    {
        devices.emplace_back(l, l);
    }

    return devices;
}