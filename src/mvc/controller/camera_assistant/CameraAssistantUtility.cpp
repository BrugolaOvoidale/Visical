#include "CameraAssistantUtility.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <UtilityFunctions.hpp>
#include <camera/ICameraInfo.hpp>


TaskResultP<std::vector<CameraAssistantModel::CameraDTO>> CameraAssistantUtility::loadCamerasConfig(const std::string& path)
{
    std::vector<CameraAssistantModel::CameraDTO> cameras;

    try
    {
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
        if (!j.contains("cameras"))
        {
            return {
                std::nullopt,
                { "Missing 'cameras' section in config", Log::Level::LVL_ERROR}
            };
        }

        const auto& c = j["cameras"];
        if (!c.is_array())
        {
            return {
                std::nullopt,
                { "Invalid JSON format: expected array", Log::Level::LVL_ERROR}
            };
        }

        for (const auto& cam : c)
        {
            CameraAssistantModel::CameraDTO camera;
            camera.id = cam.at("id");
            camera.camInterface = UtilityFunctions::enumFromString<CameraInterface>(cam.at("interface")).value();
            camera.paramsPersistence = cam.at("params_persistence") ;
            camera.behaviour = UtilityFunctions::enumFromString<CameraAssistantModel::CameraBehaviour>(cam.at("behaviour")).value();
            cameras.push_back(camera);
        }
    }
    catch (const nlohmann::json::parse_error& e)
    {
        return {
            std::nullopt,
            { "JSON parse error in " + path + ": " + e.what(), Log::Level::LVL_ERROR}
        };
    }
    catch (const nlohmann::json::type_error& e)
    {
        return {
            std::nullopt,
            { "JSON parse error in " + path + ": " + e.what(), Log::Level::LVL_ERROR}
        };
    }
    catch (const nlohmann::json::out_of_range& e)
    {
        return {
            std::nullopt,
            { "Missing required field in " + path + ": " + e.what(), Log::Level::LVL_ERROR}
        };
    }

    return cameras;
}

TaskResult CameraAssistantUtility::saveCamerasConfig(
    const std::string& path,
    const std::vector<CameraAssistantModel::CameraDescriptorInfo>& cameras)
{
    try
    {
        nlohmann::json j;

        for (const auto& descr : cameras)
        {
            j["cameras"].push_back({
                {"id", descr.camInfo->serialNumber()},
                {"interface", UtilityFunctions::stringFromEnum<CameraInterface>(descr.camInfo->interfaceType()).value()},
                {"params_persistence", descr.camInfo->areParametersPersistent()},
                {"behaviour", UtilityFunctions::stringFromEnum<CameraAssistantModel::CameraBehaviour>(descr.behaviour).value()}
            });
		}

        // Write to file
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
            { "JSON parse error in " + path + ": " + e.what(), Log::Level::LVL_ERROR}
        };
    }
    catch (const nlohmann::json::out_of_range& e)
    {
        return {
            false,
            { "Missing required field in " + path + ": " + e.what(), Log::Level::LVL_ERROR}
        };
    }

    return true;
}

TaskResultP<CameraAssistantUtility::Config> CameraAssistantUtility::loadCameraAssistantConfig(const std::string& path)
{
    CameraAssistantUtility::Config cfg;

    try
    {
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
        if (!j.contains("discovery"))
        {
            return {
                std::nullopt,
                { "Missing 'discovery' section in config", Log::Level::LVL_ERROR}
            };
        }

        const auto& d = j["discovery"];

        cfg.autoDiscovery = d.at("auto_discovery");
        cfg.discoveryBehaviour = UtilityFunctions::enumFromString<CameraAssistantModel::CameraBehaviour>(d.at("discovery_behaviour")).value();
        cfg.cameraSource = UtilityFunctions::enumFromString<CameraAssistantView::Source>(d.at("camera_source")).value();
        cfg.cameraListPath = d.at("camera_list_path");
    }
    catch (const nlohmann::json::parse_error& e)
    {
        return {
            std::nullopt,
            { "JSON parse error in " + path + ": " + e.what(), Log::Level::LVL_ERROR}
        };
    }
    catch (const nlohmann::json::type_error& e)
    {
        return {
            std::nullopt,
            { "JSON parse error in " + path + ": " + e.what(), Log::Level::LVL_ERROR}
        };
    }
    catch (const nlohmann::json::out_of_range& e)
    {
        return {
            std::nullopt,
            { "Missing required field in " + path + ": " + e.what(), Log::Level::LVL_ERROR}
        };
    }
    catch (const std::exception& e)
    {
        return {
            std::nullopt,
            { e.what(), Log::Level::LVL_ERROR}
        };
    }

    return cfg;
}