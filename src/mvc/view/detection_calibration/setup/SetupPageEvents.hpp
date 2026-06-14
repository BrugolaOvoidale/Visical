#pragma once
#include <wx/event.h>


// Payload: none
wxDECLARE_EVENT(GUI_SETUP_LOAD_CONFIG, wxCommandEvent);

// Payload: none
wxDECLARE_EVENT(GUI_SETUP_SAVE_CONFIG, wxCommandEvent);

// Payload: none
wxDECLARE_EVENT(GUI_SETUP_LOAD_MODEL_PARAMS, wxCommandEvent);

// Payload: none
wxDECLARE_EVENT(GUI_SETUP_SAVE_MODEL_PARAMS, wxCommandEvent);

// Payload: none
wxDECLARE_EVENT(GUI_AUTOMATIC_MODE, wxCommandEvent);

// Payload: none
wxDECLARE_EVENT(GUI_FROM_HARDWARE_MODE, wxCommandEvent);

// Payload: none
wxDECLARE_EVENT(GUI_FREE_MODE, wxCommandEvent);

