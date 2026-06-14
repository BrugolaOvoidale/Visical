#pragma once
#include <wx/event.h>


// Payload: none
wxDECLARE_EVENT(GUI_CAM_ASSIST_LOAD_SETTINGS, wxCommandEvent);

// Payload: none
wxDECLARE_EVENT(GUI_CAM_ASSIST_SAVE_SETTINGS, wxCommandEvent);

// Payload: none
wxDECLARE_EVENT(GUI_CAM_ASSIST_SAVE_SETTINGS_AS, wxCommandEvent);

// Payload: wxString -> selected behaviour
wxDECLARE_EVENT(GUI_CHANGE_DISCOVERY_BEHAVIOUR, wxCommandEvent);

// Payload: none
wxDECLARE_EVENT(GUI_LOAD_CAMERAS_FILE, wxCommandEvent);

// Payload: none
wxDECLARE_EVENT(GUI_AUTO_DISCOVERY, wxCommandEvent);

// Payload: none
wxDECLARE_EVENT(GUI_DISCOVER_INTERFACE, wxCommandEvent);

// Payload: none
wxDECLARE_EVENT(GUI_SAVE_CAMERAS_FILE, wxCommandEvent);

// Payload: wxString -> selected category
wxDECLARE_EVENT(GUI_CHANGE_CATEGORY, wxCommandEvent);

// Payload: wxString -> selected visbility
wxDECLARE_EVENT(GUI_CHANGE_VISIBILITY, wxCommandEvent);

// Payload: int -> camera persistence mode [0, 1]
wxDECLARE_EVENT(GUI_CHANGE_PARAMS_PERSISTENCE, wxCommandEvent);
