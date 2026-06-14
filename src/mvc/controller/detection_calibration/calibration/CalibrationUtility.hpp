#pragma once
#include "../common/CalibrationStageUtility.hpp"


// Forward declarations
class CalibrationResult;


/**
 * @brief Handles side-functions of the Calibration module.
 */
class CalibrationUtility : public CalibrationStageUtility
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    CalibrationUtility() = default;

    ~CalibrationUtility() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /**
     * @brief Serializes and saves a calibration result to the disk.
     *
     * @param path The filesystem path (including filename and extension) where the result will be stored.
     * @param calibRes The calibration data structure to be saved.
     * @return TaskResult A status object indicating success or details of any failure encountered during saving.
     */
    static TaskResult saveCalibrationResult(
        const std::string& path,
        const CalibrationResult& calibRes
    );
};
