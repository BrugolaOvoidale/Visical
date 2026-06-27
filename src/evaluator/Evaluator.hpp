#pragma once
#include <unordered_set>
#include "IEvaluator.hpp"
#include "EvaluationResult.hpp"
#include "PluginBase.hpp"


/**
 * @brief Template-based scheduler for executing plugin evaluation logic in dependency order.
 *
 * The Evaluator manages a collection of plugins and is responsible for three things only:
 *   - Maintaining the plugin registry (registration, enable/disable, parameter updates).
 *   - Resolving execution order via topological sort of declared plugin dependencies.
 *   - Invoking enabled plugins in that order, threading producer results downstream.
 *
 * The Evaluator does NOT inspect, validate, or act on the content of plugin results.
 * It does NOT short-circuit execution based on whether a dependency succeeded or failed.
 * Each plugin is solely responsible for self-diagnosing its inputs and producing an
 * appropriate result.
 * 
 * Execution contract:
 *   - Only enabled plugins are executed.
 *   - Plugins are executed in topological order; all declared dependencies of a plugin
 *     are guaranteed to have been executed before it.
 *   - A disabled dependency will simply be absent and the plugin is expected to handle
 *     this case itself.
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
     * 
     * All its dependencies are computed, but only that specific plugin result is returned.
     * 
     * @param object The data object to analyze.
     * @param pluginId The unique identifier of the target plugin.
     * @return An EvaluationResult containing only the specific plugin's output.
     */
    EvaluationResult<T> evaluate(
        const T& object,
        const std::string& pluginId
    );

    /**
     * @brief Updates an existing evaluation result by re-running a specific plugin subgraph.
     * @param evalObject The current evaluation result to be modified.
     * @param rootPluginId The Id of the plugin to re-execute, along with its subgraph.
     * @return A new EvaluationResult with the updated data for the specified plugin.
     */
    EvaluationResult<T> updateEvaluation(
        const EvaluationResult<T>& evalObject,
        const std::string& rootPluginId
    );

private:
    // Internal graph representation
    struct PluginGraph{
        // parallel arrays, index 0..n-1
        std::vector<std::shared_ptr<PluginBase<T>>> plugins;
        std::unordered_map<std::string, size_t>     idToIndex;
        std::vector<std::vector<size_t>>            dependents;  // forward edges
        std::vector<size_t>                         inDegree;

        size_t size() const { return plugins.size(); }
    };

    // Snapshots every enabled plugin into a PluginGraph.
    // Pass includeId to force-include one plugin even if disabled.
    PluginGraph buildGraph(const std::string& includeId = "") const;

    // DFS backwards from startId: returns the set of indices that are
    // transitive dependencies of startId, including startId itself.
    std::unordered_set<size_t> collectAncestors(
        const PluginGraph& g,
        const std::string& startId
    ) const;

    // BFS/DFS forwards from startId: returns the set of indices that
    // transitively depend on startId, including startId itself.
    std::unordered_set<size_t> collectDescendants(
        const PluginGraph& g,
        const std::string& startId
    ) const;

    // Runs Kahn's wave algorithm over subgraph (a subset of g's indices).
    // seedResults pre-populates results for nodes outside the subgraph
    // (used by updateEvaluation to carry forward unchanged assessments).
    //
    // Thread-safety: the results map is pre-populated with nullptr for every
    // node in the subgraph so no rehashing occurs during parallel writes.
    std::unordered_map<std::string, std::shared_ptr<PluginResult>> KahnExecute(
        const PluginGraph& g,
        const std::unordered_set<size_t>& subgraph,
        const T& object,
        std::unordered_map<std::string, std::shared_ptr<PluginResult>> results = {}
    ) const;
};

#include "Evaluator.inl"
