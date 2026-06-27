#pragma once
#include "Evaluator.hpp"
#include <stdexcept>
#include <execution>
#include <stack>
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
    const PluginGraph g = buildGraph();

    // Full graph
    std::unordered_set<size_t> all;
    all.reserve(g.size());
    for (size_t i = 0; i < g.size(); ++i)
    {
        all.insert(i);
    }

    std::unordered_map<std::string, std::shared_ptr<PluginResult>> results = KahnExecute(g, all, object);

    // Merge back into vector
    std::vector<std::shared_ptr<PluginResult>> assessments(g.size());
    for (size_t i = 0; i < g.size(); ++i)
    {
        assessments[i] = results.at(g.plugins[i]->id());
    }

    return EvaluationResult<T>(object, std::move(assessments));
}

template<typename T>
EvaluationResult<T> Evaluator<T>::evaluate(
    const T& object,
    const std::string& pluginId)
{
    const PluginGraph g = buildGraph();

    // Validate before doing any work
    {
        auto it = g.idToIndex.find(pluginId);
        if (it == g.idToIndex.end())
            throw std::runtime_error("Plugin Id not found: " + pluginId);
        // If it's absent from the graph it was disabled. buildGraph skips disabled plugins
    }

    const std::unordered_set<size_t> subgraph = collectAncestors(g, pluginId);
    std::unordered_map<std::string, std::shared_ptr<PluginResult>> results = KahnExecute(g, subgraph, object);

    return EvaluationResult<T>(object, { results.at(pluginId) });
}

template<typename T>
EvaluationResult<T> Evaluator<T>::updateEvaluation(
    const EvaluationResult<T>& evalResult,
    const std::string& rootPluginId)
{
    // Build graph including rootPluginId even if it was just disabled
    const PluginGraph g = buildGraph(rootPluginId);

    auto it = g.idToIndex.find(rootPluginId);
    if (it == g.idToIndex.end())
        throw std::runtime_error("Plugin not found: " + rootPluginId);

    bool rootIsDisabled = false;
    {
        std::shared_lock lock(pluginsMutex_);
        auto ctxIt = pluginCtxs_.find(rootPluginId);
        rootIsDisabled = (ctxIt != pluginCtxs_.end() && !ctxIt->second->isPluginEnabled());
    }

    // Seed with all old assessments so unchanged nodes are already present
    std::unordered_map<std::string, std::shared_ptr<PluginResult>> seed;
    for (const auto& a : evalResult.assessments())
        seed[a->plugin()->id()] = a;

    if (rootIsDisabled)
        seed.erase(rootPluginId);

    std::unordered_set<size_t> subgraph = collectDescendants(g, rootPluginId);

    // If root is disabled, remove it from the execution set.
    // Its dependents will still run but will receive nullptr for this plugin's result.
    if (rootIsDisabled)
        subgraph.erase(it->second);

    std::unordered_map<std::string, std::shared_ptr<PluginResult>> results = KahnExecute(g, subgraph, evalResult.object(), std::move(seed));

    // Merge back into vector
    std::vector<std::shared_ptr<PluginResult>> newVec;
    newVec.reserve(results.size());
    for (const auto& [_, a] : results)
    {
        if (a) newVec.push_back(a);
    }

    return EvaluationResult<T>(evalResult.object(), std::move(newVec));
}

///////////////////////////////////////////////////////////

template<typename T>
typename Evaluator<T>::PluginGraph Evaluator<T>::buildGraph(const std::string& includeId) const
{
    PluginGraph g;

    {
        std::shared_lock lock(pluginsMutex_);
        g.plugins.reserve(pluginCtxs_.size());

        for (const auto& [id, ctx] : pluginCtxs_)
        {
            if (!ctx->isPluginEnabled() && id != includeId)
                continue;

            g.idToIndex[id] = g.plugins.size();
            g.plugins.push_back(
                std::static_pointer_cast<PluginBase<T>>(ctx->getPlugin()));
        }
    }

    const size_t n = g.plugins.size();
    g.dependents.resize(n);
    g.inDegree.resize(n, 0);

    for (size_t i = 0; i < n; ++i)
    {
        for (const auto& dep : g.plugins[i]->dependencies())
        {
            auto it = g.idToIndex.find(dep);
            if (it == g.idToIndex.end()) continue;

            const size_t depIdx = it->second;
            g.dependents[depIdx].push_back(i);
            ++g.inDegree[i];
        }
    }

    return g;
}

template<typename T>
std::unordered_set<size_t> Evaluator<T>::collectAncestors(
    const PluginGraph& g,
    const std::string& startId) const
{
    // Build reverse edges locally (cheap for subgraph work)
    const size_t n = g.size();
    std::vector<std::vector<size_t>> predecessors(n);
    for (size_t i = 0; i < n; ++i)
    {
        for (size_t dep : g.dependents[i])  // g.dependents[i] = "i must finish before dep"
        {
            predecessors[dep].push_back(i);  // so predecessor of dep is i
        }
    }


    std::unordered_set<size_t> visited;
    std::stack<size_t> stack;
    stack.push(g.idToIndex.at(startId));

    while (!stack.empty())
    {
        size_t idx = stack.top(); stack.pop();

        if (!visited.insert(idx).second) continue;

        for (size_t pred : predecessors[idx])
        {
            stack.push(pred);
        }
    }

    return visited;
}

template<typename T>
std::unordered_set<size_t> Evaluator<T>::collectDescendants(
    const PluginGraph& g,
    const std::string& startId) const
{
    std::unordered_set<size_t> visited;
    std::stack<size_t> stack;
    stack.push(g.idToIndex.at(startId));

    while (!stack.empty())
    {
        size_t idx = stack.top(); stack.pop();

        if (!visited.insert(idx).second) continue;

        for (size_t dep : g.dependents[idx])
        {
            stack.push(dep);
        }
    }

    return visited;
}

template<typename T>
std::unordered_map<std::string, std::shared_ptr<PluginResult>> Evaluator<T>::KahnExecute(
    const PluginGraph& g,
    const std::unordered_set<size_t>& subgraph,
    const T& object,
    std::unordered_map<std::string, std::shared_ptr<PluginResult>> results) const
{
    const size_t m = subgraph.size();

    // Pre-populate every subgraph slot so parallel writes never rehash
    for (size_t i : subgraph)
    {
        results.emplace(g.plugins[i]->id(), nullptr);
    }

    // Subgraph-local in-degree (only edges whose source is also in subgraph)
    std::unordered_map<size_t, size_t> inDeg;
    inDeg.reserve(m);
    for (size_t i : subgraph)
    {
        size_t deg = 0;
        for (const auto& dep : g.plugins[i]->dependencies())
        {
            auto it = g.idToIndex.find(dep);
            if (it != g.idToIndex.end() && subgraph.count(it->second))
                ++deg;
        }
        inDeg[i] = deg;
    }

    std::vector<size_t> ready;
    ready.reserve(m);
    for (size_t i : subgraph)
    {
        if (inDeg[i] == 0)
            ready.push_back(i);
    }

    size_t executed = 0;
    while (!ready.empty())
    {
        std::for_each(std::execution::par, ready.begin(), ready.end(),
            [&](size_t i)
            {
                const auto& plugin = g.plugins[i];
                results[plugin->id()] = plugin->execute(object, results);
            });

        executed += ready.size();

        std::vector<size_t> nextReady;
        for (size_t i : ready)
        {
            for (size_t dependent : g.dependents[i])
            {
                if (subgraph.count(dependent) && --inDeg[dependent] == 0)
                    nextReady.push_back(dependent);
            }
        }

        ready = std::move(nextReady);
    }

    if (executed != m)
        throw std::runtime_error("Cycle detected in plugin dependencies");

    return results;
}
