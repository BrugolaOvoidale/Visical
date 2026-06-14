#include "PluginResult.hpp"


PluginResult::PluginResult(
    const std::shared_ptr<PluginInfo>& plugin,
    const std::string& message,
    double score,
    EvaluationSeverity severity,
    const std::shared_ptr<PluginDebugResult>& debugResult)
    : plugin_(plugin),
    message_(message),
    score_(score),
    severity_(severity),
    debugResult_(debugResult)
{
}

//////////////////////////////////////////////////////////

double PluginResult::score() const
{
    return score_;
}

const std::string& PluginResult::message() const
{
    return message_;
}

EvaluationSeverity PluginResult::severity() const
{
    return severity_;
}

const std::shared_ptr<PluginDebugResult>& PluginResult::debugResult() const
{
    return debugResult_;
}

const std::shared_ptr<PluginInfo>& PluginResult::plugin() const
{
    return plugin_;
}