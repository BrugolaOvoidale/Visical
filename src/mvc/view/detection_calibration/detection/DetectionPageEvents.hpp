#pragma once
#include "../common/board/BoardEvent.hpp"


// Payload: none
wxDECLARE_EVENT(GUI_LOAD_IMAGE, wxCommandEvent);

// Payload: none
wxDECLARE_EVENT(GUI_SAVE_IMAGE, wxCommandEvent);

// Payload: none
wxDECLARE_EVENT(GUI_SNAP_CAMERA, wxCommandEvent);

// Payload: none
wxDECLARE_EVENT(GUI_LIVE_CAMERA, wxCommandEvent);

// Payload: int [0, 1] -> if button check
wxDECLARE_EVENT(GUI_AUTO_CAPTURE, wxCommandEvent);

// Payload: none
wxDECLARE_EVENT(GUI_DET_REMOVE_BOARD, BoardEvent);

// Payload: none
wxDECLARE_EVENT(GUI_DET_REMOVE_ALL_BOARDS, wxCommandEvent);

// Payload: none
wxDECLARE_EVENT(GUI_ACCEPT_DATASET, wxCommandEvent);

// Payload: none
wxDECLARE_EVENT(GUI_CAMERA_ASSISTANT, wxCommandEvent);
