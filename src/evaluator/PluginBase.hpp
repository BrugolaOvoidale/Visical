#pragma once
#include <concepts>
#include "IPlugin.hpp"
#include "PluginResult.hpp"
#include "PluginDebugResult.hpp"


/**
 * @brief Base template class for all plugins processing objects of type T.
 * 
 * This class implements the common logic for plugin and result
 * generation.
 * 
 * @tparam T The input type that this plugin is designed to analyze.
 */
template<typename T>
class PluginBase : public IPlugin
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------
    
    ~PluginBase() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /**
     * @brief Public entry point for running the plugin logic.
     * @param object The input data to analyze.
     * @return A shared_ptr to a PluginResult containing scores and status.
	 * @throws std::invalid_argument if the parameters are out of bounds.
     */
    std::shared_ptr<PluginResult> execute(const T& object) const;

protected:
    // Protected constructor to ensure instantiation via factory methods or subclasses.
    PluginBase(
        const std::string& id,
        const std::string& name,
        const std::string& description,
        double threshold = DEFAULT_THRESHOLD
    );

    /**
     * @brief Core processing logic to be implemented by the derived class.
     * @param object The input data.
     * @return The specific result of the analysis.
     */
    virtual std::shared_ptr<PluginResult> executeImpl(const T& object) const = 0;

    /** @brief Validates that the plugin's parameters are within logical bounds. */
    virtual void validateParameters() const = 0;

    /** @brief Returns a result indicating a processing error occurred. */
    std::shared_ptr<PluginResult> executionFailed(const std::string& message) const;
};

#include "PluginBase.inl"
