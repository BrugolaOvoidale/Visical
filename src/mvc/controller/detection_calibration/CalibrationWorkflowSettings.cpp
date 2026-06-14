#include "CalibrationWorkflowSettings.hpp"
#include <fstream>
#include <nlohmann/json.hpp>
#include <optional>
#include <UtilityFunctions.hpp>


TaskResultP<CalibrationWorkflowSettings::Loaded> CalibrationWorkflowSettings::loadSettings()
{
    Loaded loadedData;

    try
    {
        std::string path = getSettingsPath();
        if (path.empty())
        {
            path = getDefaultSettingsPath();

            if (path.empty())
            {
                return {
                    std::nullopt,
                    { "Path is empty", Log::Level::LVL_ERROR}
                };
            }

            setSettingsPath(path);
        }

        std::ifstream file(path);
        if (!file.is_open())
        {
            return {
                std::nullopt,
                { "Failed to open file: " + path, Log::Level::LVL_ERROR}
            };
        }

        nlohmann::json j = nlohmann::json::parse(file);

        // Check if required keys exist
        if (!j.contains("setup"))
        {
            return {
                std::nullopt,
                { "Missing 'setup' section in config", Log::Level::LVL_ERROR}
            };
        }

        const auto& d = j["setup"];

        setModelParamsPath(d.at("setup_params_path"));
        loadedData.calibMode = UtilityFunctions::enumFromString<CameraIntrinsics::Mode>(d.at("calibration_mode")).value();
    }
    catch (const nlohmann::json::parse_error& e)
    {
        return {
            std::nullopt,
            { "JSON parse error in " + getSettingsPath() + ": " + e.what(), Log::Level::LVL_ERROR}
        };
    }
    catch (const nlohmann::json::type_error& e)
    {
        return {
            std::nullopt,
            { "JSON parse error in " + getSettingsPath() + ": " + e.what(), Log::Level::LVL_ERROR}
        };
    }
    catch (const nlohmann::json::out_of_range& e)
    {
        return {
            std::nullopt,
            { "Missing required field in " + getSettingsPath() + ": " + e.what(), Log::Level::LVL_ERROR}
        };
    }
    catch (const std::exception& e)
    {
        return {
            std::nullopt,
            { "Missing required field in " + getSettingsPath() + ": " + e.what(), Log::Level::LVL_ERROR}
        };
    }

    return loadedData;
}

TaskResult CalibrationWorkflowSettings::saveSettings(CameraIntrinsics::Mode calibMode) const
{
    try
    {
        nlohmann::json j;

        // Build calibration section
        j["setup"] = {
            {"setup_params_path", getModelParamsPath()},
            {"calibration_mode", UtilityFunctions::stringFromEnum(calibMode).value()}
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