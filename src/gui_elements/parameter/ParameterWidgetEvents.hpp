#include "ParameterChangedEvent.hpp"

// Payload:
//      - wxString -> parameter Id;
//      - wxString -> category Id
//      - custom payload -> std::variant ('std::monostate' in case of meaningless value)
wxDECLARE_EVENT(GUI_SET_PARAM_VALUE, ParameterChangedEvent);

// Payload: wxString -> parameter Id ; wxString -> category Id
wxDECLARE_EVENT(GUI_SELECT_PARAM, ParameterEvent);

// Payload: wxString -> parameter Id ; wxString -> category Id
wxDECLARE_EVENT(GUI_RESET_PARAM, ParameterEvent);