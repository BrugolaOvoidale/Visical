#pragma once
#include <wx/event.h>


// Payload: none
wxDECLARE_EVENT(GUI_LOAD_SETTINGS, wxCommandEvent);

// Payload: none
wxDECLARE_EVENT(GUI_SAVE_SETTINGS, wxCommandEvent);

// Payload: none
wxDECLARE_EVENT(GUI_LOAD_MODEL_PARAMS, wxCommandEvent);

// Payload: none
wxDECLARE_EVENT(GUI_SAVE_MODEL_PARAMS, wxCommandEvent);

// Payload: int [0, 1] -> if button check
wxDECLARE_EVENT(GUI_DRAW_BOARD, wxCommandEvent);

// Payload: int [0, 1] -> if button check
wxDECLARE_EVENT(GUI_DRAW_MARKS, wxCommandEvent);

// Payload: int [0, 1] -> if button check
wxDECLARE_EVENT(GUI_DRAW_WCS, wxCommandEvent);
