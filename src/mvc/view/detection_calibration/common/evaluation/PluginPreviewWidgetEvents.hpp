#pragma once
#include <wx/event.h>


// Payload: wxString -> plugin Id
wxDECLARE_EVENT(GUI_SELECT_PLUGIN_PREVIEW, wxCommandEvent);

// Payload: wxString -> plugin Id
wxDECLARE_EVENT(GUI_ADD_PLUGIN, wxCommandEvent);

// Payload: wxString -> plugin Id
wxDECLARE_EVENT(GUI_REMOVE_PLUGIN, wxCommandEvent);