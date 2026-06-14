#include "PluginContext.hpp"
#include <parameter/ParameterUtils.hpp>
#include "IPlugin.hpp"
#include "ParameterToPluginTranslator.hpp"


PluginContext::PluginContext(
    const std::shared_ptr<IPlugin>& plugin,
    const std::vector<std::shared_ptr<Parameter>>& parameters,
    const std::shared_ptr<ParameterToPluginTranslator>& pluginTranslator,
    bool enabled)
    : plugin_(plugin), pluginTranslator_(pluginTranslator), enabled_(enabled)
{
	initParameters(parameters);

    applyParametersToPlugin();
}

PluginContext::PluginContext(
    const std::shared_ptr<IPlugin>& plugin,
    const std::vector<std::shared_ptr<ParameterInfo>>& parameters,
    const std::shared_ptr<ParameterToPluginTranslator>& pluginTranslator,
    bool enabled)
    : plugin_(plugin), pluginTranslator_(pluginTranslator), enabled_(enabled)
{
    initParameters(parameters);

    applyParametersToPlugin();
}

/////////////////////////////////////////////////////

std::shared_ptr<PluginContext> PluginContext::create(
    const std::shared_ptr<IPlugin>& plugin,
    const std::vector<std::shared_ptr<Parameter>>& parameters,
    const std::shared_ptr<ParameterToPluginTranslator>& pluginTranslator,
    bool enabled)
{
    if (parameters.empty())
    {
        throw std::runtime_error("parameters is empty");
	}

    if (!plugin)
    {
        throw std::runtime_error("plugin is nullptr");
	}

    struct Enabler : PluginContext
    {
        Enabler(
            const std::shared_ptr<IPlugin>& plugin,
            const std::vector<std::shared_ptr<Parameter>>& parameters,
            const std::shared_ptr<ParameterToPluginTranslator>& pluginTranslator,
            bool enabled)
            : PluginContext(plugin, parameters, pluginTranslator, enabled)
        {
        }
    };

    return std::make_shared<Enabler>(
        plugin,
        parameters,
		pluginTranslator,
        enabled
    );
}

std::shared_ptr<PluginContext> PluginContext::create(
    const std::shared_ptr<IPlugin>& plugin,
    const std::vector<std::shared_ptr<ParameterInfo>>& parameters,
    const std::shared_ptr<ParameterToPluginTranslator>& pluginTranslator,
    bool enabled)
{
    if (parameters.empty())
    {
        throw std::runtime_error("parameters is empty");
    }

    if (!plugin)
    {
        throw std::runtime_error("plugin is nullptr");
    }

    struct Enabler : PluginContext
    {
        Enabler(
            const std::shared_ptr<IPlugin>& plugin,
            const std::vector<std::shared_ptr<ParameterInfo>>& parameters,
            const std::shared_ptr<ParameterToPluginTranslator>& pluginTranslator,
            bool enabled)
            : PluginContext(plugin, parameters, pluginTranslator, enabled)
        {
        }
    };

    return std::make_shared<Enabler>(
        plugin,
        parameters,
        pluginTranslator,
        enabled
    );
}

std::shared_ptr<PluginContext> PluginContext::create(
    const std::shared_ptr<IPlugin>& plugin,
    bool enabled)
{
    struct Enabler : PluginContext
    {
        Enabler(
            const std::shared_ptr<IPlugin>& plugin,
            bool enabled)
            : PluginContext(plugin, std::vector<std::shared_ptr<Parameter>>{}, nullptr, enabled)
        {
        }
    };

    return std::make_shared<Enabler>(
        plugin,
        enabled
    );
}

/////////////////////////////////////////////////////

const std::shared_ptr<IPlugin>& PluginContext::getPlugin() const
{
    return plugin_;
}

std::shared_ptr<PluginContextInfo> PluginContext::getPluginContextView() const
{
    return std::make_shared<PluginContextInfoImpl>(
        std::static_pointer_cast<const PluginContext>(shared_from_this())
    );
}

void PluginContext::enablePlugin(bool enable)
{
    enabled_ = enable;
}

void PluginContext::disablePlugin()
{
    enablePlugin(false);
}

bool PluginContext::isPluginEnabled() const
{
    return enabled_;
}

TaskResult PluginContext::setParameter(
    const std::string& paramId,
    const std::string& categoryId,
    const std::string& value)
{
	TaskResult result = ParameterOwner::setParameter(paramId, categoryId, value);

    if (result.isSuccess())
        applyParametersToPlugin();

    return result;
}

TaskResult PluginContext::setParameter(
    const std::string& paramId,
    const std::string& categoryId,
    int value)
{
    TaskResult result = ParameterOwner::setParameter(paramId, categoryId, value);

    if (result.isSuccess())
        applyParametersToPlugin();

    return result;
}

TaskResult PluginContext::setParameter(
    const std::string& paramId,
    const std::string& categoryId,
    double value)
{
    TaskResult result = ParameterOwner::setParameter(paramId, categoryId, value);

    if (result.isSuccess())
        applyParametersToPlugin();

    return result;
}

TaskResult PluginContext::setParameter(
    const std::string& paramId,
    const std::string& categoryId,
    bool value)
{
    TaskResult result = ParameterOwner::setParameter(paramId, categoryId, value);

    if (result.isSuccess())
        applyParametersToPlugin();

    return result;
}

TaskResult PluginContext::resetParameter(
    const std::string& paramId,
    const std::string& categoryId)
{
    TaskResult result = ParameterOwner::resetParameter(paramId, categoryId);

    if (result.isSuccess())
        applyParametersToPlugin();

    return result;
}

/////////////////////////////////////////////////////

std::shared_ptr<const ParameterOwner> PluginContext::getSharedParameterOwner() const
{
    return std::static_pointer_cast<const ParameterOwner>(
        std::static_pointer_cast<const PluginContext>(shared_from_this())
    );
}

void PluginContext::applyParametersToPlugin()
{
    if (pluginTranslator_)
        pluginTranslator_->applyParametersToPlugin(plugin_, getFilteredParams());
}


/////////////////////////////////////////////////////


PluginContext::PluginContextInfoImpl::PluginContextInfoImpl(const std::shared_ptr<const PluginContext>& pluginContext)
    : ParameterOwnerInfoImpl(pluginContext),
    PluginContextInfo(pluginContext->getPlugin()->getPluginView()), pluginContext_(pluginContext)
{
    if (!pluginContext_)
    {
        throw std::runtime_error("pluginContext is nullptr");
    }
}

/////////////////////////////////////////////////////

bool PluginContext::PluginContextInfoImpl::isPluginEnabled() const
{
    return pluginContext_->isPluginEnabled();
}