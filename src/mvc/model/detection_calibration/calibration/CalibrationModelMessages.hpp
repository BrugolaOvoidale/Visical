#pragma once
#include "CalibrationResult.hpp"
#include <message_bus/types/MessageP.hpp>
#include <message_bus/types/MessageTaskP.hpp>


// Calibrate camera
REGISTER_MESSAGE(MessageTaskP<CalibrationResult>, MSG_CALIBRATE_CAMERA);

//
REGISTER_MESSAGE(Message, MSG_CALIBRATION_PARAMETERS_CHANGED);

//
REGISTER_MESSAGE(MessageP<std::vector<EvaluatedCalibratedBoardEntry>>, MSG_CALIB_BOARD_REEVALUATION);

//
REGISTER_MESSAGE(MessageP<EvaluatedCameraModel>, MSG_CAM_MODEL_REEVALUATION);
