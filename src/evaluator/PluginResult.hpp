#pragma once
#include <memory>
#include <string>
#include "EvaluationDefs.hpp"


// Forward declarations
class PluginInfo;
class PluginDebugResult;


/**
 * @brief Encapsulates the outcome of a plugin's evaluation process.
 *
 * This class holds the informations resulting a plugin execution. It also optionally contains diagnostic
 * data via PluginDebugResult.
 */
class PluginResult
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    /**
     * @brief Constructs a new PluginResult object.
     * 
     * @param plugin A shared pointer to the metadata of the plugin that produced this result.
     * @param message A human-readable summary of the result.
     * @param score The numerical value calculated by the plugin.
     * @param severity The importance or error level of this result.
     * @param debugResult Optional shared pointer to extended debug/diagnostic information.
     */
    PluginResult(
        const std::shared_ptr<PluginInfo>& plugin,
        const std::string& message,
        double score,
        EvaluationSeverity severity,
        const std::shared_ptr<PluginDebugResult>& debugResult = nullptr
    );

    ~PluginResult() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /** @brief Returns the numerical score of the evaluation. */
    double score() const;

    /** @brief Returns the human-readable description of the result. */
    const std::string& message() const;

    /** @brief Returns the severity level assigned to this result. */
    EvaluationSeverity severity() const;

    /**
     * @brief Returns a shared pointer to the debug information.
     * @return shared_ptr to PluginDebugResult, or nullptr if no debug data was provided.
     */
    const std::shared_ptr<PluginDebugResult>& debugResult() const;

    /** @brief Returns the metadata of the plugin associated with this result (TODO: at the moment of the execution). */
    const std::shared_ptr<PluginInfo>& plugin() const;

private:
    // Associated plugin metadata.
    std::shared_ptr<PluginInfo> plugin_;

    // Detailed result message.
    std::string message_;

    // Quantitative evaluation score.
    double score_;

    // Severity level (see EvaluationDefs.h).
    EvaluationSeverity severity_;

    // Optional diagnostic data.
    std::shared_ptr<PluginDebugResult> debugResult_;
};
