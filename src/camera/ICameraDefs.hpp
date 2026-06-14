#pragma once
#include <vector>
#include <enum/EnumTraits.hpp>

/**
 * @brief Supported communication protocols for cameras.
 * This enum defines the backend driver used to communicate with the physical device.
 */
enum class CameraInterface {
#ifdef WITH_GENICAM
    /**
     * @brief GenICam standard via Aravis. Only available if
     * the project is compiled with the WITH_GENICAM flag.
     */
    ARAVIS_GENICAM,
#endif
    OPENCV           ///< Standard OpenCV VideoCapture interface.
};

/**
 * @brief Specialization of EnumTraits for CameraInterface.
 * Used for automated string-to-enum conversion.
 */
template <>
struct EnumTraits<CameraInterface> {
    static inline const std::vector<std::pair<CameraInterface, const char*>> values{
#ifdef WITH_GENICAM
        {CameraInterface::ARAVIS_GENICAM, "aravis_genicam"},
#endif
        {CameraInterface::OPENCV, "openCV"},
    };
};

/**
 * @brief List of valid (defined) camera interfaces for discovery and iteration.
 */
static const std::vector<CameraInterface> CAM_INTERFACES{
#ifdef WITH_GENICAM
    CameraInterface::ARAVIS_GENICAM,
#endif
    CameraInterface::OPENCV
};

/**
 * @brief Defines how the camera triggers and retrieves frames.
 */
enum class AcquisitionMode {
    SINGLE_FRAME,   ///< Capture exactly one frame then stop.
    MULTI_FRAME,    ///< Capture a specific burst of frames.
    CONTINUOUS      ///< Stream frames indefinitely until a manual stop command.
};
