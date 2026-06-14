#include "CalibrationUtility.hpp"
#include <fstream>
#include <mvc/model/detection_calibration/calibration/CalibrationResult.hpp>


TaskResult CalibrationUtility::saveCalibrationResult(
	const std::string& path,
	const CalibrationResult& calibRes)
{
	if (path.empty())
		return false;

    try
    {
        nlohmann::ordered_json j;

        // Serialize cameraMatrixRes and distortionModelRes.
        // writeParams will walk each param's category path and nest values automatically.
        writeParams(j, calibRes.cameraMatrix());
        writeParams(j, calibRes.distortionModel());

        // reprojErr_ is a plain double, assign it directly.
        j["cameraModel"]["reprojectionError"] = calibRes.evaluatedCameraModel().object().reprojectionError();

        std::ofstream file(path);
        if (!file.is_open())
        {
            return {
                false,
                { "Failed to open file: " + path, Log::Level::LVL_ERROR}
            };
        }

        file << j.dump(4);

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
    catch (const std::exception& e)
    {
        return {
            false,
            { "Exception in " + path + ": " + e.what(), Log::Level::LVL_ERROR }
        };
    }

    return true;
}
