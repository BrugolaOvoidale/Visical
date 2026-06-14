#include "PluginContextInfo.hpp"
#include <stdexcept>


PluginContextInfo::PluginContextInfo(const std::shared_ptr<PluginInfo>& pluginInfo) : pluginInfo_(pluginInfo)
{
    if (!pluginInfo)
    {
        throw std::runtime_error("pluginInfo is nullptr");
    }
}

/////////////////////////////////////////////////////////////////////////////////

const std::shared_ptr<PluginInfo>& PluginContextInfo::getPluginInfo() const
{
    return pluginInfo_;
};