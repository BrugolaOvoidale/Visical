#pragma once
#include <mvc/model/camera_assistant/CameraAssistantModel.hpp>
#include <mvc/view/camera_assistant/CameraAssistantView.hpp>


/**
 * @brief Utility class for managing Camera Assistant configurations and persistence.
 *
 * This class provides helper methods to serialize and deserialize camera data
 * and application settings. It bridges the Model-View definitions with the file system.
 */
class CameraAssistantUtility
{
public:
    //-----------------------------------------------------------------------------
    // Structs
    //-----------------------------------------------------------------------------

    /**
     * @brief Aggregated configuration settings for the Camera Assistant module.
     */
    struct Config
    {
        // Scan for hardware at startup.
        bool autoDiscovery;

        // How the module handles new camera discovery.
        CameraAssistantModel::CameraBehaviour discoveryBehaviour;

        // The defined input source for the camera feed.
        CameraAssistantView::Source cameraSource;

        // Filesystem path to the JSON list of cameras
        std::string cameraListPath;
	};


    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    CameraAssistantUtility() = default;

    ~CameraAssistantUtility() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /**
     * @brief Loads the list of cameras from a configuration file.
     * @param path The file path to load from.
     * @return A TaskResult containing a vector of Camera DTOs.
     */
    static TaskResultP<std::vector<CameraAssistantModel::CameraDTO>> loadCamerasConfig(const std::string& path);

    /**
     * @brief Saves the current camera descriptors to a file.
     * @param path The destination file path.
     * @param cameras A vector of descriptors to be serialized.
     * @return A TaskResult indicating success or failure of the write operation.
     */
    static TaskResult saveCamerasConfig(
        const std::string& path,
        const std::vector<CameraAssistantModel::CameraDescriptorInfo>& cameras
    );

    /**
     * @brief Loads the global Camera Assistant settings.
     * @param path The file path to the configuration file.
     * @return A TaskResultP containing the populated Config struct.
     */
    static TaskResultP<Config> loadCameraAssistantConfig(const std::string& path);
};