#pragma once
#include <message_bus/types/MessageP.hpp>
#include <message_bus/types/MessageTaskP.hpp>
#include "DetectionResultMap.hpp"
#include "board_sequence/EvaluatedBoardSequence.hpp"


// Board analysis from file image
REGISTER_MESSAGE(MessageTask, MSG_BOARD_FROM_IMAGE);
////////////////////////////////////////////////////////////////////

// Board analysis from camera snapshot
REGISTER_MESSAGE(MessageTask, MSG_BOARD_FROM_SNAP);
////////////////////////////////////////////////////////////////////

// Board analysis from camera live acquistion
REGISTER_MESSAGE(MessageTask, MSG_BOARD_FROM_LIVE);
////////////////////////////////////////////////////////////////////

// Board re-analysis from dataset
REGISTER_MESSAGE(MessageTask, MSG_BOARD_REDETECTION);
////////////////////////////////////////////////////////////////////

// Single board re-evaluate by board check
REGISTER_MESSAGE(MessageTask, MSG_BOARD_REEVALUATION);
////////////////////////////////////////////////////////////////////

// Board stored
REGISTER_MESSAGE(MessageP<DetectionResultMap::Entry>, MSG_BOARD_STORED);
////////////////////////////////////////////////////////////////////

// Board analysis from re-run
REGISTER_MESSAGE(MessageP<DetectionResultMap::Entry>, MSG_BOARD_UPDATE);
////////////////////////////////////////////////////////////////////

// Board sequence re-evaluate by board sequence check
REGISTER_MESSAGE(MessageP<std::shared_ptr<EvaluatedBoardSequence>>, MSG_BOARD_SEQUENCE_REEVALUATED);
////////////////////////////////////////////////////////////////////

//
REGISTER_MESSAGE(Message, MSG_PATTERN_TYPE_CHANGED);
////////////////////////////////////////////////////////////////////
