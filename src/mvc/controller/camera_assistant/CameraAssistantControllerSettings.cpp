#include "CameraAssistantControllerSettings.hpp"
#include <fstream>
#include <nlohmann/json.hpp>


CameraAssistantControllerSettings::CameraAssistantControllerSettings()
{
    std::string path = getSettingsPath();
    if (path.empty())
    {
        path = getDefaultSettingsPath();

        if (!path.empty())
        {
            setSettingsPath(path);
        }
    }

    loadSettings();
}

///////////////////////////////////////////////////////////////////////////////

TaskResult CameraAssistantControllerSettings::loadSettings()
{
    try
    {
        std::string path = getSettingsPath();

        std::ifstream file(path);
        if (!file.is_open())
        {
            return {
                false,
                { "Failed to open file: " + path, Log::Level::LVL_ERROR}
            };
        }

        nlohmann::json j = nlohmann::json::parse(file);

        // Check if required keys exist
        if (!j.contains("discovery"))
        {
            return {
                false,
                { "Missing 'discovery' section in config", Log::Level::LVL_ERROR}
            };
        }

        const auto& c = j["discovery"];

        setCamerasConfigPath(c.at("camera_list_path"));
    }
    catch (const nlohmann::json::parse_error& e)
    {
        return {
            false,
            { "JSON parse error in " + getSettingsPath() + ": " + e.what(), Log::Level::LVL_ERROR}
        };
    }
    catch (const nlohmann::json::type_error& e)
    {
        return {
            false,
            { "JSON parse error in " + getSettingsPath() + ": " + e.what(), Log::Level::LVL_ERROR}
        };
    }
    catch (const nlohmann::json::out_of_range& e)
    {
        return {
            false,
            { "Missing required field in " + getSettingsPath() + ": " + e.what(), Log::Level::LVL_ERROR}
        };
    }

    return true;
}

TaskResult CameraAssistantControllerSettings::saveSettings(
    bool autoDiscovery,
    const std::string& discoveryBehaviour,
    const std::string& cameraSource) const
{
    try
    {
        nlohmann::json j;

        // Build calibration section
        j["discovery"] = {
            {"auto_discovery", autoDiscovery},
            {"discovery_behaviour", discoveryBehaviour},
            {"camera_source", cameraSource},
            {"camera_list_path", getCamerasConfigPath()}
        };


        // Write to file
        const std::string& path = getSettingsPath();
        std::ofstream file(path);
        if (!file.is_open())
        {
            return {
                false,
                { "Failed to open file: " + path, Log::Level::LVL_ERROR}
            };
        }

        file << j.dump(4); // Pretty print with 4-space indentation
        file.close();

        if (file.fail())
        {
            return {
                false,
                { "Failed to write file: " + path, Log::Level::LVL_ERROR}
            };
        }
    }
    catch (const nlohmann::json::type_error& e)
    {
        return {
            false,
            { "JSON parse error in " + getSettingsPath() + ": " + e.what(), Log::Level::LVL_ERROR}
        };
    }
    catch (const std::exception& e)
    {
        return {
            false,
            { "Exception while saving config in " + getSettingsPath() + ": " + e.what(), Log::Level::LVL_ERROR }
        };
    }

    return true;
}

void CameraAssistantControllerSettings::setCamerasConfigPath(const std::string& path)
{
    camerasConfigPath_ = path;
}

const std::string& CameraAssistantControllerSettings::getCamerasConfigPath() const
{
    return camerasConfigPath_;
}

///////////////////////////////////////////////////////////////////////////////

std::string CameraAssistantControllerSettings::getDefaultSettingsPathImpl() const
{
    return getModuleDirImpl() +
        PATH_SEP +
        getDefaultSettingsFileName();
}

const std::string& CameraAssistantControllerSettings::getModuleDirImpl() const
{
    return CONFIG_DIR;
}

const std::string& CameraAssistantControllerSettings::getModelParamsFilePathImpl() const
{
    return PARAMS_FILE;
}

const std::string& CameraAssistantControllerSettings::getDefaultSettingsFileNameImpl() const
{
    return SETTINGS_FILE;
}