#pragma once
#include <message_bus/types/MessageTaskP.hpp>
#include "CameraState.hpp"


static const std::string CAM_SERIAL{ "cam_serial" };

//
REGISTER_MESSAGE(MessageTask, MSG_SHUTDOWN_CAMERA);
//

//
REGISTER_MESSAGE(MessageTaskP<CameraState>, MSG_CHECK_CAMERA);
//

//
REGISTER_MESSAGE(MessageTask, MSG_CONNECT_CAMERA);
//

//
REGISTER_MESSAGE(MessageTask, MSG_DISCONNECT_CAMERA);
//

//
REGISTER_MESSAGE(MessageTask, MSG_START_GRABBING);
//

//
REGISTER_MESSAGE(MessageTask, MSG_STOP_GRABBING);
//

//
REGISTER_MESSAGE(Message, MSG_ACQUIRED_FRAME);
//

//
REGISTER_MESSAGE(Message, MSG_ACQUIRED_FRAME_FAIL);
//
