#pragma once
#include <task_result/TaskResult.hpp>
#include <mvc/view/detection_calibration/detection/DetectionPage.hpp>
#include "../common/CalibrationStageControllerSettings.hpp"


/**
 * @brief Manages configuration paths and plugin settings for the Detection Stage.
 *
 * This class extends CalibrationStageControllerSettings to provide specific handling for
 * board and sequence plugins.
 */
class DetectionSettings : public CalibrationStageControllerSettings
{
public:
    //-----------------------------------------------------------------------------
    // Structs and constants
    //-----------------------------------------------------------------------------

    /**
     * @brief Data structure containing the configuration state loaded from disk.
     */
    struct Loaded {
        DetectionPage::ImageSource imgSrc;
        bool autoCapture;
        bool drawBoard;
        bool drawMarks;
        bool drawWCS;
    };

    static inline const std::string CONFIG_DIR{ "Detection" };
    static inline const std::string SETTINGS_FILE{ "detection_settings.json" };
    static inline const std::string PARAMS_FILE{ "detection_params.json" };


    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    DetectionSettings();

    ~DetectionSettings() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /**
     * @brief Loads the detection settings from the default JSON path.
     * @return A TaskResult containing the Loaded struct if successful,
     * or error details on failure.
     */
    TaskResultP<Loaded> loadSettings();

    /**
     * @brief Persists the provided settings to the local JSON configuration file.
     * @param imgSrc Image source.
     * @param autoCapture flag for auto capture results.
     * @param drawBoard flag for board contour visualization.
     * @param drawMarks flag for mark visualization.
     * @param drawWCS flag for coordinate system visualization.
     * @return TaskResult indicating success or the specific cause of persistence failure.
     */
    TaskResult saveSettings(
        DetectionPage::ImageSource imgSrc,
        bool autoCapture,
        bool drawBoard,
        bool drawMarks,
        bool drawWCS
    ) const;

private:
    const std::string& getModuleDirImpl() const override;

    const std::string& getModelParamsFilePathImpl() const override;

    const std::string& getDefaultSettingsFileNameImpl() const override;
};
