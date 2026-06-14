#pragma once
#include "../BaseControllerSettings.hpp"
#include <task_result/TaskResult.hpp>


/**
 * @brief Manages the persistence and retrieval of configuration for the Camera Assistant module.
 *
 * It extends BaseControllerSettings to provide module-specific directory structures
 * and file naming conventions.
 */
class CameraAssistantControllerSettings : public BaseControllerSettings
{
public:
    //-----------------------------------------------------------------------------
    // Constants
    //-----------------------------------------------------------------------------

    static inline const std::string CONFIG_DIR{ "CameraAssistant" };
    static inline const std::string SETTINGS_FILE{ "camera_assistant_settings.json" };
    static inline const std::string PARAMS_FILE{ "camera_assistant_params.json" };


    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    CameraAssistantControllerSettings();

    ~CameraAssistantControllerSettings() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /**
     * @brief Loads settings from the default filesystem location.
     * @return TaskResult indicating success or describing the failure.
     */
    TaskResult loadSettings();

    /**
     * @brief Updates and persists the camera assistant configuration.
     * @param autoDiscovery Policy for discovery cameras on startup.
     * @param discoveryBehaviour Method for detecting new hardware.
     * @param cameraSource Identifier for the primary camera input.
     * @return TaskResult indicating the outcome of the disk write operation.
     */
    TaskResult saveSettings(
        bool autoDiscovery,
        const std::string& discoveryBehaviour,
        const std::string& cameraSource
    ) const;

    /** @brief Sets the filesystem path where camera-specific configurations are stored. */
    void setCamerasConfigPath(const std::string& path);

    /** @brief Gets the current path used for camera configurations. */
    const std::string& getCamerasConfigPath() const;

private:
    std::string getDefaultSettingsPathImpl() const;

    const std::string& getModuleDirImpl() const override;

    const std::string& getModelParamsFilePathImpl() const override;

    const std::string& getDefaultSettingsFileNameImpl() const override;

private:
    // The stored path for camera configurations.
    std::string camerasConfigPath_;
};