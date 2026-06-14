#pragma once
#include "PluginBase.hpp"


template<typename T>
PluginBase<T>::PluginBase(
    const std::string& id,
    const std::string& name,
    const std::string& description,
    double threshold)
    : IPlugin(id, name, description, threshold)
{
}

///////////////////////////////////////////////////////////

template<typename T>
std::shared_ptr<PluginResult> PluginBase<T>::execute(const T& object) const
{
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
        return executeImpl(object);
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
        0.0,
        EvaluationSeverity::FAILED
    );
}