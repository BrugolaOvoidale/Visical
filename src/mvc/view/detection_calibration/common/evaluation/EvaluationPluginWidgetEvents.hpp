#include <gui_elements/parameter/ParameterChangedEvent.hpp>


// Payload: wxString -> plugin Id
wxDECLARE_EVENT(GUI_SELECT_EVAL_PLUGIN, wxCommandEvent);

// Payload: wxString -> plugin Id
wxDECLARE_EVENT(GUI_RIGHT_CLICK_EVAL_PLUGIN, wxCommandEvent);

// Payload: wxString -> plugin Id ; int threshold
wxDECLARE_EVENT(GUI_SET_THRESHOLD_EVAL_PLUGIN, wxCommandEvent);

// Payload: wxString -> plugin Id ; int [0, 1] -> if button check
wxDECLARE_EVENT(GUI_ENABLE_DISABLE_EVAL_PLUGIN, wxCommandEvent);

// Payload: 
//      - wxString -> plugin Id;
//      - wxString -> parameter Id;
//      - wxString -> category Id
wxDECLARE_EVENT(GUI_RESET_PLUGIN_PARAM, ParameterEvent);

// Payload:
//      - wxString -> plugin Id;
//      - wxString -> parameter Id;
//      - wxString -> category Id
//      - custom payload -> std::variant ('std::monostate' in case of meaningless value)
wxDECLARE_EVENT(GUI_SET_PLUGIN_PARAM_VALUE, ParameterChangedEvent);

// Payload: int [0, 1] -> if button check
wxDECLARE_EVENT(GUI_SHOW_DEB_RESULT, wxCommandEvent);

// Payload: none
wxDECLARE_EVENT(GUI_OPEN_PLUGIN_PARAMS, wxCommandEvent);