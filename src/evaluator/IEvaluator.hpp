#pragma once
#include <variant>
#include <shared_mutex>
#include <unordered_map>
#include <task_result/TaskResult.hpp>


// Forward declarations
class PluginContext;
class PluginContextInfo;


/**
 * @brief Abstract base class for managing and coordinating plugin-based evaluations.
 *
 * This class provides a thread-safe registry for PluginContext objects.
 * It allows for dynamic registration, configuration of thresholds, and parameter
 * tuning of individual plugins via a unified interface.
 */
class IEvaluator
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    IEvaluator() = default;

    virtual ~IEvaluator() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /**
     * @brief Registers a new plugin context into the evaluator.
     * @param pluginCtx Shared pointer to the plugin context to register.
     * @return true if registration was successful, false otherwise.
     */
    virtual bool registerPlugin(const std::shared_ptr<PluginContext>& pluginCtx) = 0;

    /**
     * @brief Removes a plugin from the registry by its unique identifier.
     * @param pluginId The unique string Id of the plugin to remove.
     */
    void unregisterPlugin(const std::string& pluginId);

    /** @brief Clears all registered plugins from the evaluator. */
    void unregisterAllPlugins();

    /**
     * @brief Updates the activation threshold for a specific plugin.
     * @param pluginId Unique Id of the plugin.
     * @param threshold The new threshold value.
     * @return The previous threshold value if the plugin exists, std::nullopt otherwise.
     */
    std::optional<double> setPluginThreshold(
        const std::string& pluginId,
        double threshold
    );

    /**
     * @brief Updates the threshold for all currently registered plugins.
     * @param threshold The new global threshold value.
     */
    void setAllPluginsThreshold(double threshold);

    /**
     * @brief Enables or disables a specific plugin.
     * @param pluginId Unique Id of the plugin.
     * @param enable True to enable, false to disable.
     * @return The previous state if successful, std::nullopt if plugin not found.
     */
    std::optional<bool> enablePlugin(
        const std::string& pluginId,
        bool enable
    );

    /** @brief Checks if a plugin with the given Id is currently registered. */
    bool hasPlugin(const std::string& pluginId) const;

    /** @brief Retrieves a list of metadata/info for all registered plugins. */
    std::vector<std::shared_ptr<PluginContextInfo>> getAllRegisteredPlugins() const;

    /**
     * @brief Retrieves info for a specific registered plugin.
     * @return Pointer to the info object, or nullptr if not found.
     */
    std::shared_ptr<PluginContextInfo> getRegisteredPlugin(const std::string& pluginId) const;

    /**
     * @brief Modifies a specific parameter within a plugin's configuration.
     * @param pluginId Id of the target plugin.
     * @param paramId Id of the parameter to change.
     * @param categoryId Category under which the parameter is grouped.
     * @param value The new value.
     * @return TaskResult indicating success or specific failure reasons.
     */
    TaskResult setPluginParameter(
        const std::string& pluginId,
        const std::string& paramId,
        const std::string& categoryId,
        const std::variant<std::monostate, std::string, int, double, bool>& value
    );

    /**
     * @brief Resets a plugin parameter to its default value.
     * @param pluginId Id of the target plugin.
     * @param paramId Id of the parameter.
     * @param categoryId Category of the parameter.
     * @return TaskResult indicating result of the reset operation.
     */
    TaskResult resetPluginParameter(
        const std::string& pluginId,
        const std::string& paramId,
        const std::string& categoryId
    );

protected:
    /**
     * @brief Internal helper to retrieve the full context of a plugin.
     * @param pluginId The unique IDdto search for.
     * @return Shared pointer to the context or nullptr if not found.
     */
    std::shared_ptr<PluginContext> getPluginContext(const std::string& pluginId) const;

protected:
    // Mutex to protect concurrent access to the plugin registry.
    mutable std::shared_mutex pluginsMutex_;

    // Internal storage mapping plugin Ids to their contexts.
    std::unordered_map<std::string, std::shared_ptr<PluginContext>> pluginCtxs_;
};
