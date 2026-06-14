#pragma once
#include "../BaseControllerSettings.hpp"
#include <task_result/TaskResult.hpp>
#include <calibration/CameraIntrinsics.hpp>


/**
 * @brief Manages the persistence and retrieval of configuration for the calibration workflow.
 *
 * It extends BaseControllerSettings to provide module-specific directory structures
 * and file naming conventions.
 */
class CalibrationWorkflowSettings : public BaseControllerSettings
{
public:
    //-----------------------------------------------------------------------------
    // Structs and constants
    //-----------------------------------------------------------------------------

    /**
     * @brief Data structure containing the configuration state loaded from disk.
     */
    struct Loaded {
        CameraIntrinsics::Mode calibMode;    ///< The selected calibration mode.
    };

    static inline const std::string CONFIG_DIR{ "Setup" };
    static inline const std::string SETTINGS_FILE{ "setup_settings.json" };
    static inline const std::string PARAMS_FILE{ "setup_params.json" };


    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    CalibrationWorkflowSettings() = default;

    ~CalibrationWorkflowSettings() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /**
     * @brief Loads settings from the default filesystem location.
     * @return A TaskResult containing the Loaded struct if successful,
     * or error details on failure.
     */
    TaskResultP<Loaded> loadSettings();

    /**
     * @brief Updates and persists the calibration workflow configuration.
     * @param calibMode The selected calibration mode.
     * @return TaskResult indicating the outcome of the disk write operation.
     */
    TaskResult saveSettings(CameraIntrinsics::Mode calibMode) const;

private:
    std::string getDefaultSettingsPathImpl() const;

    const std::string& getModuleDirImpl() const override { return CONFIG_DIR; }

    const std::string& getModelParamsFilePathImpl() const override { return PARAMS_FILE; }

    const std::string& getDefaultSettingsFileNameImpl() const override { return SETTINGS_FILE; }
};
