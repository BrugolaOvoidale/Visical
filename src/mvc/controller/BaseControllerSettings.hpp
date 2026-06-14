#pragma once
#include <string>


/**
 * @brief Abstract base class for managing module-specific configuration and model paths.
 *
 * This class provides a unified interface for resolving filesystem paths for settings
 * and model parameters. It uses the "Template Method" pattern, where derived classes
 * must implement the 'Impl' methods to provide module-specific naming conventions.
 */
class BaseControllerSettings
{
public:
    //-----------------------------------------------------------------------------
    // Constants
    //-----------------------------------------------------------------------------

    /** @brief Platform-independent path separator (defaults to "/"). */
    static inline const std::string PATH_SEP{ "/" };


    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    BaseControllerSettings() = default;

    /** @brief Virtual destructor to ensure proper cleanup of derived classes. */
    virtual ~BaseControllerSettings() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /**
     * @brief Retrieves the root configuration directory for the application.
     * @return A string representing the base config path.
     */
    static std::string getConfigDirectory();

    /** @brief Constructs the full default path to the settings file. */
    std::string getDefaultSettingsPath() const;

    /** @brief Constructs the full default path to the model parameters file. */
    std::string getDefaultModelParamsPath() const;

    /** @brief Returns the module-specific filename for settings. */
    std::string getDefaultSettingsFileName() const;

    /** @brief Manually overrides the settings file path. */
    void setSettingsPath(const std::string& path);

    /** @brief Gets the currently active settings file path. */
    const std::string& getSettingsPath() const;

    /** @brief Manually overrides the model parameters file path. */
    void setModelParamsPath(const std::string& path);

    /** @brief Gets the currently active model parameters file path. */
    const std::string& getModelParamsPath() const;

    /**
     * @brief Interface for derived classes to provide their specific module directory.
     * @return Reference to a string containing the module directory name.
     */
    virtual const std::string& getModuleDirImpl() const = 0;

    /**
     * @brief Interface for derived classes to provide their specific model filename.
     * @return Reference to a string containing the model filename.
     */
    virtual const std::string& getModelParamsFilePathImpl() const = 0;

    /**
     * @brief Interface for derived classes to provide their default settings filename.
     * @return Reference to a string containing the settings filename.
     */
    virtual const std::string& getDefaultSettingsFileNameImpl() const = 0;

private:
    // The resolved or overridden path to model parameters.
    std::string modelParamsPath_;

    // The resolved or overridden path to settings.
    std::string settingsPath_;
};