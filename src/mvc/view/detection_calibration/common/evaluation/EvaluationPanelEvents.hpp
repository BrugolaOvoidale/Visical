#include <wx/event.h>


// Per-board events

// Payload: int [0, 1] -> if button check
wxDECLARE_EVENT(GUI_BOARD_SHOW_DEB_RESULT, wxCommandEvent);

// Payload: none
wxDECLARE_EVENT(GUI_BOARD_LOAD_PLUGINS, wxCommandEvent);

// Payload: none
wxDECLARE_EVENT(GUI_BOARD_SAVE_PLUGINS, wxCommandEvent);

// Payload: wxString -> pluginId
wxDECLARE_EVENT(GUI_BOARD_ADD_PLUGIN, wxCommandEvent);

// Payload: wxString -> pluginId
wxDECLARE_EVENT(GUI_BOARD_REMOVE_PLUGIN, wxCommandEvent);

// Payload: int -> threshold
wxDECLARE_EVENT(GUI_BOARD_GLOBAL_THR, wxCommandEvent);


// Per-sequence events

// Payload: int [0, 1] -> if button check
wxDECLARE_EVENT(GUI_SEQUENCE_SHOW_DEB_RESULT, wxCommandEvent);

// Payload: none
wxDECLARE_EVENT(GUI_SEQUENCE_LOAD_PLUGINS, wxCommandEvent);

// Payload: none
wxDECLARE_EVENT(GUI_SEQUENCE_SAVE_PLUGINS, wxCommandEvent);

// Payload: wxString -> pluginId
wxDECLARE_EVENT(GUI_SEQUENCE_ADD_PLUGIN, wxCommandEvent);

// Payload: wxString -> pluginId
wxDECLARE_EVENT(GUI_SEQUENCE_REMOVE_PLUGIN, wxCommandEvent);

// Payload: int -> threshold
wxDECLARE_EVENT(GUI_SEQUENCE_GLOBAL_THR, wxCommandEvent);