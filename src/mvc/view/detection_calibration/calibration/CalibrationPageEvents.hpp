#pragma once
#include <wx/event.h>


// Payload: none
wxDECLARE_EVENT(GUI_SAVE_CALIBRATION, wxCommandEvent);

// Payload: none
wxDECLARE_EVENT(GUI_START_CALIBRATION, wxCommandEvent);

// Payload: int [0, 1] -> if button check
wxDECLARE_EVENT(GUI_UNDISTORT, wxCommandEvent);
