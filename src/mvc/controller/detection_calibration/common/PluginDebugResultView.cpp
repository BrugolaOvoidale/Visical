#include "PluginDebugResultView.hpp"


PluginDebugResultView::PluginDebugResultView(
    wxString pluginId,
    wxImage debugImage)
    : pluginId_(std::move(pluginId)),
    debugImage_(std::move(debugImage))
{
}

/////////////////////////////////////////////

const wxString& PluginDebugResultView::pluginId() const
{
    return pluginId_;
}

const wxImage& PluginDebugResultView::debugImage() const
{
    return debugImage_;
}