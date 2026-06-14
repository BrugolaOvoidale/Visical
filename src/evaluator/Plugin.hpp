#pragma once
#include "PluginBase.hpp"


/** * @brief Sentinel type used when a plugin does not provide debug information. */
struct NoDebug {};

/**
 * @brief Base template for specific plugin implementations.
 * 
 * This class extends PluginBase and provides helper methods for generating
 * PluginResult objects based on scoring and severity thresholds.
 * 
 * @tparam T The specific plugin type.
 * @tparam DebugT The type containing debug-specific data. Defaults to NoDebug.
 */
template<typename T, typename DebugT = NoDebug>
class Plugin : public PluginBase<T>
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    ~Plugin() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /**
     * @brief Checks if this plugin is capable of producing debug information.
     * @return true if DebugT is not NoDebug, false otherwise.
     */
    bool supportsDebugResults() const override;

protected:
    // Protected constructor to ensure instantiation via factory methods or subclasses.
    Plugin(
        const std::string& id,
        const std::string& name,
        const std::string& description,
        double threshold = IPlugin::DEFAULT_THRESHOLD
    );

    /**
     * @brief Maps a numerical score to a severity level based on the plugin threshold.
     * @param score The raw value to evaluate.
     * @return The resulting EvaluationSeverity.
     */
    EvaluationSeverity evaluateScore(double score) const;

    /**
     * @brief Creates a standard result object.
     * @param score The numerical outcome of the plugin execution.
     * @return A shared pointer to the populated PluginResult.
     */
    std::shared_ptr<PluginResult> executionResult(double score) const;

    /**
     * @brief Creates a detailed result object including diagnostic data.
     * @param quality The numerical outcome of the plugin execution.
     * @param debugResult Shared pointer to the supplementary debug information.
     * @return A shared pointer to the populated PluginResult.
     */
    std::shared_ptr<PluginResult> executionResult(
        double quality,
        const std::shared_ptr<PluginDebugResult>& debugResult
    ) const;

    /**
     * @brief Creates a result indicating that the plugin logic could not be performed.
     * @param message Description of why the plugin was not applicable to the current input.
     * @return A shared pointer to a PluginResult with a 'Not Applicable' status.
     */
    std::shared_ptr<PluginResult> executionNotApplicable(const std::string& message) const;

    /**
     * @brief Pure virtual method to be implemented by subclasses to provide context-aware messages.
     * @param severity The calculated severity level.
     * @return A string message explaining the result to the end-user.
     */
    virtual std::string getMessageForSeverity(EvaluationSeverity severity) const = 0;
};

#include "Plugin.inl"
