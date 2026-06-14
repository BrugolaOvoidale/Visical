#include "DetectionSettings.hpp"
#include <fstream>
#include <nlohmann/json.hpp>
#include <UtilityFunctions.hpp>


DetectionSettings::DetectionSettings()
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

TaskResultP<DetectionSettings::Loaded> DetectionSettings::loadSettings()
{
    DetectionSettings::Loaded res;

    try
    {
        std::string path = getSettingsPath();

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
        if (!j.contains("detection"))
        {
            return {
                std::nullopt,
                { "Missing 'detection' section in config", Log::Level::LVL_ERROR}
            };
        }

        const auto& d = j["detection"];

        setModelParamsPath(d.at("detection_params_path"));

        setBoardPluginsPath(d.at("single_board_plugins_path"));

        setSequencePluginsPath(d.at("board_sequence_plugins_path"));

        res.imgSrc = UtilityFunctions::enumFromString<DetectionPage::ImageSource>(d.at("image_source")).value();

        res.drawBoard = d.at("draw_board");

        res.drawMarks = d.at("draw_marks");

        res.drawWCS = d.at("draw_WCS");
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
            { "Exception while saving config in " + getSettingsPath() + ": " + e.what(), Log::Level::LVL_ERROR }
        };
    }

    return res;
}

TaskResult DetectionSettings::saveSettings(
    DetectionPage::ImageSource imgSrc,
    bool drawBoard,
    bool drawMarks,
    bool drawWCS) const
{
    try
    {
        nlohmann::json j;

        // Build detection section
        j["detection"] = {
            {"image_source", UtilityFunctions::stringFromEnum(imgSrc).value()},
            {"draw_board", drawBoard},
            {"draw_marks", drawMarks},
            {"draw_WCS", drawWCS},
            {"detection_params_path", getModelParamsPath()},
            {"single_board_plugins_path", getBoardPluginsPath()},
            {"board_sequence_plugins_path", getSequencePluginsPath()}
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
            { "Exception while saving config in " + getSettingsPath()+ ": " + e.what(), Log::Level::LVL_ERROR }
        };
    }

    return true;
}

///////////////////////////////////////////

const std::string& DetectionSettings::getModuleDirImpl() const
{
    return CONFIG_DIR;
}

const std::string& DetectionSettings::getModelParamsFilePathImpl() const
{
    return PARAMS_FILE;
}

const std::string& DetectionSettings::getDefaultSettingsFileNameImpl() const
{
    return SETTINGS_FILE;
}