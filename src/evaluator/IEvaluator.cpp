#include "IEvaluator.hpp"
#include <stdexcept>
#include "PluginContext.hpp"
#include "IPlugin.hpp"


void IEvaluator::unregisterPlugin(const std::string& pluginId)
{
    pluginCtxs_.erase(pluginId);
}

void IEvaluator::unregisterAllPlugins()
{
    std::unique_lock lock(pluginsMutex_);
    pluginCtxs_.clear();
}

std::optional<bool> IEvaluator::enablePlugin(const std::string& pluginId, bool enable)
{
    std::unique_lock lock(pluginsMutex_);
    auto p = pluginCtxs_.find(pluginId);
    if (p == pluginCtxs_.end()) return std::nullopt;

    bool oldValue = p->second->isPluginEnabled();

    p->second->enablePlugin(enable);

    return oldValue;
}

bool IEvaluator::hasPlugin(const std::string& pluginId) const
{
    std::shared_lock lock(pluginsMutex_);
    auto p = pluginCtxs_.find(pluginId);
    if (p == pluginCtxs_.end()) return false;

    return true;
}

std::vector<std::shared_ptr<PluginContextInfo>> IEvaluator::getAllRegisteredPlugins() const
{
    std::vector<std::shared_ptr<PluginContextInfo>> registeredChecks;

    std::shared_lock lock(pluginsMutex_);
    for (const auto& [_, pluginCtx] : pluginCtxs_)
    {
        registeredChecks.push_back(pluginCtx->getPluginContextView());
    }

    return registeredChecks;
}

std::shared_ptr<PluginContextInfo> IEvaluator::getRegisteredPlugin(const std::string& pluginId) const
{
    std::shared_lock lock(pluginsMutex_);
    auto p = pluginCtxs_.find(pluginId);
    if (p == pluginCtxs_.end()) return nullptr;

    return p->second->getPluginContextView();
}

TaskResult IEvaluator::setPluginParameter(
    const std::string& pluginId,
    const std::string& paramId,
    const std::string& categoryId,
    const std::variant<std::monostate, std::string, int, double, bool>& value)
{
    std::unique_lock lock(pluginsMutex_);
    auto p = pluginCtxs_.find(pluginId);
    if (p == pluginCtxs_.end())
        return { false, "Plugin " + paramId + " does not exist in cache" };

    TaskResult setRes;
    if (auto* stringVal = std::get_if<std::string>(&value))
    {
        setRes = p->second->setParameter(
            paramId,
            categoryId,
            (*stringVal)
        );
    }
    else if (auto* intVal = std::get_if<int>(&value))
    {
        setRes = p->second->setParameter(
            paramId,
            categoryId,
            *intVal
        );
    }
    else if (auto* doubleVal = std::get_if<double>(&value))
    {
        setRes = p->second->setParameter(
            paramId,
            categoryId,
            *doubleVal
        );
    }
    else if (auto* boolVal = std::get_if<bool>(&value))
    {
        setRes = p->second->setParameter(
            paramId,
            categoryId,
            *boolVal
        );
    }
    else
    {
        setRes = p->second->executeCommand(
            paramId,
            categoryId
        );
    }

	return setRes;
}

TaskResult IEvaluator::resetPluginParameter(
    const std::string& pluginId,
    const std::string& paramId,
    const std::string& categoryId)
{
    std::unique_lock lock(pluginsMutex_);
    auto p = pluginCtxs_.find(pluginId);
    if (p == pluginCtxs_.end())
        return { false, "Plugin " + paramId + " does not exist in cache" };

    return p->second->resetParameter(paramId, categoryId);
}

std::optional<double> IEvaluator::setPluginThreshold(const std::string& pluginId, double threshold)
{
    std::shared_lock lock(pluginsMutex_);
    auto p = pluginCtxs_.find(pluginId);
    if (p == pluginCtxs_.end()) return std::nullopt;

    double oldValue = p->second->getPlugin()->getThreshold();

    p->second->getPlugin()->setThreshold(threshold);

    return oldValue;
}

void IEvaluator::setAllPluginsThreshold(double threshold)
{
    std::unique_lock lock(pluginsMutex_);

    for (const auto& [_, ctx] : pluginCtxs_)
    {
        ctx->getPlugin()->setThreshold(threshold);
    }
}

//////////////////////////////////////////////////////////

std::shared_ptr<PluginContext> IEvaluator::getPluginContext(const std::string& pluginId) const
{
    std::shared_lock lock(pluginsMutex_);
    auto p = pluginCtxs_.find(pluginId);
    if (p == pluginCtxs_.end()) return nullptr;

    return p->second;
}