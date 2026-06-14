#include "IPlugin.hpp"
#include <stdexcept>


IPlugin::IPlugin(
    const std::string& id,
    const std::string& name,
    const std::string& description,
    double threshold)
    : id_(id), name_(name), description_(description), threshold_(threshold)
{
}

/////////////////////////////////////////////////////////////

const std::string& IPlugin::id() const
{
    return id_;
}

const std::string& IPlugin::name() const
{
    return name_;
}

const std::string& IPlugin::description() const
{
    return description_;
}

double IPlugin::getThreshold() const
{
    return threshold_;
}

void IPlugin::setThreshold(double thr)
{
    threshold_ = thr;
}

std::shared_ptr<PluginInfo> IPlugin::getPluginView() const
{
    return std::make_shared<PluginInfoImpl>(shared_from_this());
}


/////////////////////////////////////////////////////////////


IPlugin::PluginInfoImpl::PluginInfoImpl(const std::shared_ptr<const IPlugin>& plugin) : plugin_(plugin)
{
    if (!plugin_)
    {
        throw std::invalid_argument("plugin is nullptr");
    }
}

/////////////////////////////////////////////////////////////

const std::string& IPlugin::PluginInfoImpl::id() const
{
    return plugin_->id();
}

const std::string& IPlugin::PluginInfoImpl::name() const
{
    return plugin_->name();
}

const std::string& IPlugin::PluginInfoImpl::description() const
{
    return plugin_->description();
}

double IPlugin::PluginInfoImpl::getThreshold() const
{
    return plugin_->getThreshold();
}

bool IPlugin::PluginInfoImpl::supportsDebugResults() const
{
    return plugin_->supportsDebugResults();
}