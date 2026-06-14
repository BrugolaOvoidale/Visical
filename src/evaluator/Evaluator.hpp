#pragma once
#include "IEvaluator.hpp"
#include "EvaluationResult.hpp"


/**
 * @brief Template-based engine for executing evaluation logic via plugins.
 * 
 * This class manages a suite of plugins that perform analysis on objects of type T.
 * It supports full evaluation, targeted plugin execution, and incremental updates
 * to existing results.
 * 
 * @tparam T The type of the data object to be evaluated.
 */
template<typename T>
class Evaluator : public IEvaluator
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    Evaluator() = default;

    ~Evaluator() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /**
     * @brief Registers a new analysis plugin into the evaluator.
     * @param pluginCtx A shared pointer to the plugin context and logic.
     * @return true if the plugin was successfully registered, false if Plugin is not same T.
     */
    bool registerPlugin(const std::shared_ptr<PluginContext>& pluginCtx) override;

    /**
     * @brief Performs a complete evaluation of an object using all registered plugins.
     * @param object The data object to analyze.
     * @return An EvaluationResult containing the combined output of all plugins.
     */
    EvaluationResult<T> evaluate(const T& object);
    /**
         * @brief Evaluates an object using only a specific plugin.
         * @param object The data object to analyze.
         * @param pluginId The unique identifier of the target plugin.
         * @return An EvaluationResult containing only the specific plugin's output.
         */
    EvaluationResult<T> evaluate(
        const T& object,
        const std::string& pluginId
    );

    /**
     * @brief Updates an existing evaluation result by re-running a specific plugin.
     * @param evalObject The current evaluation result to be modified.
     * @param pluginId The Id of the plugin to re-execute.
     * @return A new EvaluationResult with the updated data for the specified plugin.
     */
    EvaluationResult<T> updateEvaluation(
        const EvaluationResult<T>& evalObject,
        const std::string& pluginId
    );

    /**
     * @brief Removes a specific plugin's data from an existing evaluation result.
     * @param evalObject The current evaluation result.
     * @param pluginId The Id of the plugin whose data should be purged.
     * @return A new EvaluationResult excluding the specified plugin's output.
     */
    EvaluationResult<T> removeEvaluation(
        const EvaluationResult<T>& evalObject,
        const std::string& pluginId
    );
};

#include "Evaluator.inl"
