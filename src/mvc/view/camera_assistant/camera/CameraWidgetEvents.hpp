#pragma once
#include <wx/event.h>


// Payload: wxString -> camera Id
wxDECLARE_EVENT(GUI_SELECT_CAMERA, wxCommandEvent);

// Payload: wxString -> camera Id
wxDECLARE_EVENT(GUI_RIGHT_CLICK_CAMERA, wxCommandEvent);

// Payload: wxString -> camera Id
wxDECLARE_EVENT(GUI_CHECK_CAMERA, wxCommandEvent);

// Payload: wxString -> camera Id
wxDECLARE_EVENT(GUI_CONNECT_CAMERA, wxCommandEvent);

// Payload: wxString -> camera Id
wxDECLARE_EVENT(GUI_DISCONNECT_CAMERA, wxCommandEvent);

// Payload: wxString -> behaviour
wxDECLARE_EVENT(GUI_CHANGE_BEHAVIOUR, wxCommandEvent);