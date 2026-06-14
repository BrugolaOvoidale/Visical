#pragma once
#include "../../BaseControllerSettings.hpp"


/**
 * @brief Manages configuration paths and plugin settings for a Calibration Stage.
 *
 * This class extends BaseControllerSettings to provide specific handling for
 * board and sequence plugins.
 */
class CalibrationStageControllerSettings : public BaseControllerSettings
{
public:
    //-----------------------------------------------------------------------------
    // Constants
    //-----------------------------------------------------------------------------

    static inline const std::string BOARD_PLUGINS{ "board_plugins.json" };
    static inline const std::string SEQUENCE_PLUGINS{ "sequence_plugins.json" };


    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    CalibrationStageControllerSettings() = default;

    ~CalibrationStageControllerSettings() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /**
     * @brief Resolves the default path for model parameters.
     * @return A full system path string.
     */
    std::string getDefaultModelParamsPath() const;

    /**
     * @brief Resolves the default path for the board plugins JSON file.
     * @return A string containing the default path based on the module directory.
     */
    std::string getDefaultBoardPluginsPath() const;

    /**
     * @brief Resolves the default path for the sequence plugins JSON file.
     * @return A string containing the default path.
     */
    std::string getDefaultSequencePluginsPath() const;

    /** @brief Overrides the current board plugins path. */
    void setBoardPluginsPath(const std::string& path);

    /** @brief Gets the current board plugins path. */
    const std::string& getBoardPluginsPath() const;

    /** @brief Overrides the current sequence plugins path. */
    void setSequencePluginsPath(const std::string& path);

    /** @brief Gets the current sequence plugins path. */
    const std::string& getSequencePluginsPath() const;

private:
    virtual const std::string& getModuleDirImpl() const = 0;

    virtual const std::string& getModelParamsFilePathImpl() const = 0;

    virtual const std::string& getDefaultSettingsFileNameImpl() const = 0;

private:
    // Path to the model parameters file.
    std::string modelParamsPath_;

    // Path to the board plugins configuration.
    std::string singleBoardPluginsPath_;

    // Path to the sequence plugins configuration.
    std::string boardSeqPluginsPath_;
};
