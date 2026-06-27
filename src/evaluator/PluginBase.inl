#pragma once
#include "PluginBase.hpp"


template<typename T>
PluginBase<T>::PluginBase(
    const std::string& id,
    const std::string& name,
    const std::string& description,
    const std::vector<std::string>& dependencies,
    double threshold)
    : IPlugin(id, name, description, dependencies, threshold)
{
}

///////////////////////////////////////////////////////////

template<typename T>
std::shared_ptr<PluginResult> PluginBase<T>::execute(
    const T& object,
    const std::unordered_map<std::string, std::shared_ptr<PluginResult>>& producersResults) const
{
    // Check dependencies
    for (const auto& id : dependencies())
    {
        auto it = producersResults.find(id);
        if (it == producersResults.end())
        {
            return executionNotApplicable("Producer " + id + " not available");
        }

        const EvaluationSeverity prodSev = it->second->severity();
        if (prodSev == EvaluationSeverity::FAILED || prodSev == EvaluationSeverity::INSUFFICIENT_DATA)
        {
            return executionNotApplicable("Producer " + id + " is not usable");
        }
    }

    // Validation
    try
    {
        validateParameters();
    }
    catch (const std::exception& ex)
    {
        return executionFailed(ex.what());
    }

    // Execution
    try
    {
        return executeImpl(object, producersResults);
    }
    catch (const std::exception& ex)
    {
        return executionFailed(ex.what());
    }
    catch (...)
    {
        return executionFailed("Unkown exception occured");
    }
}

template<typename T>
std::shared_ptr<PluginResult> PluginBase<T>::executionFailed(const std::string& message) const
{
    return std::make_shared<PluginResult>(
        getPluginView(),
        message,
        -1.0,
        EvaluationSeverity::FAILED
    );
}

template<typename T>
std::shared_ptr<PluginResult> PluginBase<T>::executionNotApplicable(const std::string& message) const
{
    return std::make_shared<PluginResult>(
        IPlugin::getPluginView(),
        message,
        -1.0,
        EvaluationSeverity::INSUFFICIENT_DATA
    );
}