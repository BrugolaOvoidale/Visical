#pragma once
#include "Evaluator.hpp"
#include <stdexcept>
#include <execution>
#include "PluginBase.hpp"
#include "PluginContext.hpp"


template<typename T>
bool Evaluator<T>::registerPlugin(const std::shared_ptr<PluginContext>& pluginCtx)
{
    std::shared_ptr<PluginBase<T>> plugin = std::dynamic_pointer_cast<PluginBase<T>>(pluginCtx->getPlugin());
    if (!plugin)
        return false; 


    std::unique_lock lock(pluginsMutex_);

    pluginCtxs_[plugin->id()] = pluginCtx;

    return true;
}

template<typename T>
EvaluationResult<T> Evaluator<T>::evaluate(const T& object)
{
   std::vector<std::pair<size_t, std::shared_ptr<PluginBase<T>>>> indexedPlugins;
   {
       std::shared_lock lock(pluginsMutex_);
       size_t i = 0;
       for (const auto& [pluginId, pluginCtx] : pluginCtxs_)
       {
           if (!pluginCtx->isPluginEnabled()) { ++i; continue; }
           indexedPlugins.emplace_back(
               i++,
               std::static_pointer_cast<PluginBase<T>>(pluginCtx->getPlugin())
           );
       }
   }

   const size_t n = indexedPlugins.size();

   std::vector<std::shared_ptr<PluginResult>> assessments(n);

   std::vector<size_t> indices(n);
   std::iota(indices.begin(), indices.end(), 0);

   std::for_each(std::execution::par, indices.begin(), indices.end(),
       [&](size_t i) {
           assessments[i] = indexedPlugins[i].second->execute(object);
       }
   );

   return EvaluationResult<T>(object, std::move(assessments));
}

template<typename T>
EvaluationResult<T> Evaluator<T>::evaluate(
    const T& object,
    const std::string& pluginId)
{
    std::shared_lock lock(pluginsMutex_);

    std::vector<std::shared_ptr<PluginResult>> assessments;

    auto it = pluginCtxs_.find(pluginId);
    if (it == pluginCtxs_.end())
    {
        throw std::runtime_error("PluginBase Id not found: " + pluginId);
    }

    std::shared_ptr<PluginContext> pluginCtx = it->second;

    if (pluginCtx->isPluginEnabled())
    {
        std::shared_ptr<PluginBase<T>> plugin = std::static_pointer_cast<PluginBase<T>>(pluginCtx->getPlugin());

        assessments.push_back(
            plugin->execute(object)
        );
    }

    return EvaluationResult<T>(
        object,
        std::move(assessments)
    );
}

template<typename T>
EvaluationResult<T> Evaluator<T>::updateEvaluation(
    const EvaluationResult<T>& evalObject,
    const std::string& pluginId)
{
    std::shared_lock lock(pluginsMutex_);

    auto it = pluginCtxs_.find(pluginId);
    if (it == pluginCtxs_.end())
    {
        throw std::runtime_error("Check Id not found: " + pluginId);
    }

    std::shared_ptr<PluginContext> pluginCtx = it->second;

    const std::vector<std::shared_ptr<PluginResult>>& oldAssessments = evalObject.assessments();

    std::vector<std::shared_ptr<PluginResult>> newAssessments;
    newAssessments.reserve(oldAssessments.size());

    for (const auto& a : oldAssessments)
    {
        if (a->plugin()->id() != pluginId)
        {
            newAssessments.push_back(a);
        }
    }

    const T& object = evalObject.object();

    if (pluginCtx->isPluginEnabled())
    {
        std::shared_ptr<PluginBase<T>> plugin = std::static_pointer_cast<PluginBase<T>>(pluginCtx->getPlugin());

        newAssessments.push_back(
            plugin->execute(object)
        );
    }

    return EvaluationResult<T>(
        object,
        std::move(newAssessments)
    );
}

template<typename T>
EvaluationResult<T> Evaluator<T>::removeEvaluation(
    const EvaluationResult<T>& evalObject,
    const std::string& pluginId)
{
    const std::vector<std::shared_ptr<PluginResult>>& oldAssessments = evalObject.assessments();

    std::vector<std::shared_ptr<PluginResult>> newAssessments;
    newAssessments.reserve(oldAssessments.size());

    for (const auto& a : oldAssessments)
    {
        if (a->plugin()->id() != pluginId)
        {
            newAssessments.push_back(a);
        }
    }

    return EvaluationResult<T>(
        evalObject.object(),
        std::move(newAssessments)
    );
}
