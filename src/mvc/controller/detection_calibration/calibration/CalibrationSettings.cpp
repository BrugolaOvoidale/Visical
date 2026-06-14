#include "CalibrationSettings.hpp"
#include <fstream>
#include <nlohmann/json.hpp>


CalibrationSettings::CalibrationSettings()
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

TaskResultP<CalibrationSettings::Loaded> CalibrationSettings::loadSettings()
{
    CalibrationSettings::Loaded res;

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
        if (!j.contains("calibration"))
        {
            return {
                std::nullopt,
                { "Missing 'calibration' section in config", Log::Level::LVL_ERROR}
            };
        }

        const auto& d = j["calibration"];

        setModelParamsPath(d.at("calibration_params_path"));

        setBoardPluginsPath(d.at("single_board_plugins_path"));

        setSequencePluginsPath(d.at("board_sequence_plugins_path"));

        res.autoCalibOnNewDataset = d.at("auto_calibration_on_new_dataset");
        
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

    return res;
}

TaskResult CalibrationSettings::saveSettings(
    bool autoCalibOnNewDataset,
    bool drawBoard,
    bool drawMarks,
    bool drawWCS) const
{
    try
    {
        nlohmann::json j;

        // Build calibration section
        j["calibration"] = {
            {"autoCalibOnNewDataset", autoCalibOnNewDataset},
            {"draw_board", drawBoard},
            {"draw_marks", drawMarks},
            {"draw_WCS", drawWCS},
            {"calibration_params_path", getModelParamsPath()},
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
    catch (const std::exception& e)
    {
        return {
            false,
            { "Exception while saving config in " + getSettingsPath() + ": " + e.what(), Log::Level::LVL_ERROR }
        };
    }

    return true;
}

///////////////////////////////////////////

const std::string& CalibrationSettings::getModuleDirImpl() const
{
    return CONFIG_DIR;
}

const std::string& CalibrationSettings::getModelParamsFilePathImpl() const
{
    return PARAMS_FILE;
}

const std::string& CalibrationSettings::getDefaultSettingsFileNameImpl() const
{
    return SETTINGS_FILE;
}