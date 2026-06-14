#pragma once
#include <memory>
#include <vector>
#include <string>


// Forward declarations
class ICamera;


/**
 * @brief Static utility class for camera discovery and instantiation.
 *
 * This class provides factory methods to create camera instances based on different
 * backends and tools to enumerate connected hardware.
 */
class CameraUtils
{
public:
    //-----------------------------------------------------------------------------
    // Structs
    //-----------------------------------------------------------------------------

    /**
	 * @brief Hold camera information for enumeration results.
     */
    struct Descriptor
    {
        // Unique identifier for the camera.
        std::string serial;

        // Unique interface identifier.
        std::string ifaceId;  
	};


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

#ifdef WITH_GENICAM
    /**
     * @brief Instantiates a GenICam-compatible camera using the Aravis backend to enumerate.
     * @param camSerial The serial number of the target camera.
     * @return A shared pointer to the camera instance or nullptr if not found.
     * 
     */
    static std::shared_ptr<ICamera> createAravisCam(const std::string& camSerial);

    /**
     * @brief Lists all GenICam cameras currently visible on the network/bus via Aravis.
     * @return A vector of descriptors for all available Aravis cameras.
     */
    static std::vector<Descriptor> enumerateAravis();
#endif
    /**
     * @brief Instantiates a camera using the OpenCV (V4L2/DirectShow) backend to enumerate.
     * @param camSerial The string representation of the device path.
     * @return A shared pointer to the camera instance, or nullptr if instantiation fails.
     */
    static std::shared_ptr<ICamera> createOpenCVCamera(const std::string& camSerial);

    /**
     * @brief Lists cameras available through the standard OpenCV backends.
     * @return A vector of descriptors for found devices.
     */
    static std::vector<Descriptor> enumerateOpenCV();
};