#pragma once
#include "Plugin.hpp"


template<typename T, typename DebugT>
Plugin<T, DebugT>::Plugin(
    const std::string& id,
    const std::string& name,
    const std::string& description,
    const std::vector<std::string>& dependencies,
    double threshold)
    : PluginBase<T>(id, name, description, dependencies, threshold)
{
}

///////////////////////////////////////////////////////////

template<typename T, typename DebugT>
bool Plugin<T, DebugT>::supportsDebugResults() const
{
    return !std::is_same_v<DebugT, NoDebug>;
}

///////////////////////////////////////////////////////////

template<typename T, typename DebugT>
inline EvaluationSeverity Plugin<T, DebugT>::evaluateScore(double score) const
{
    EvaluationSeverity severity = EvaluationSeverity::OK;

    if (score < 0.0)
    {
        severity = EvaluationSeverity::FAILED;
    }
    else if (score < IPlugin::DEFAULT_CRITICAL_THR * IPlugin::getThreshold())
    {
        severity = EvaluationSeverity::CRITICAL;
    }
    else if (score < IPlugin::getThreshold())
    {
        severity = EvaluationSeverity::WARNING;
    }

    return severity;
}

template<typename T, typename DebugT>
std::shared_ptr<PluginResult> Plugin<T, DebugT>::executionResult(double score) const
{
    EvaluationSeverity severity = evaluateScore(score);

    return std::make_shared<PluginResult>(
        IPlugin::getPluginView(),
        getMessageForSeverity(severity),
        score,
        severity
    );
}

template<typename T, typename DebugT>
std::shared_ptr<PluginResult> Plugin<T, DebugT>::executionResult(
    double score,
    const std::shared_ptr<PluginDebugResult>& debugResult) const
{
    static_assert(!std::is_same_v<DebugT, NoDebug>, "Debug result is not supported for this plugin");


    EvaluationSeverity severity = evaluateScore(score);

    return std::make_shared<PluginResult>(
        IPlugin::getPluginView(),
        getMessageForSeverity(severity),
        score,
        severity,
        debugResult
    );
}
