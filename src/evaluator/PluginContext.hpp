#pragma once
#include <parameter/ParameterOwner.hpp>
#include "PluginContextInfo.hpp"


// Forward declarations
class IPlugin;
class ParameterToPluginTranslator;


/**
 * @brief Orchestrates the lifecycle and parameter management of an evaluation plugin.
 * 
 * This class acts as a wrapper around an IPlugin, handling how parameters are
 * translated and applied to the plugin's internal state. It inherits from
 * ParameterOwner to provide a standardized interface for parameter modification.
 */
class PluginContext : public ParameterOwner, public std::enable_shared_from_this<PluginContext>
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    ~PluginContext() = default;


    //-----------------------------------------------------------------------------
    // Construction from factories
    //-----------------------------------------------------------------------------

    /**
     * @brief Creates a PluginContext for a Plugin that does have parameters.
     * @param plugin The concrete plugin implementation.
     * @param parameters List of parameters to be managed by this context.
     * @param pluginTranslator Logic to translate parameter changes to plugin calls.
     * @param enabled Initial operational state of the plugin.
     * @return A shared pointer to the newly created context.
     */
    static std::shared_ptr<PluginContext> create(
        const std::shared_ptr<IPlugin>& plugin,
        const std::vector<std::shared_ptr<Parameter>>& parameters,
        const std::shared_ptr<ParameterToPluginTranslator>& pluginTranslator,
        bool enabled = true
    );

    /**
     * @brief Creates a PluginContext for a Plugin that does have parameters.
     * @param plugin The concrete plugin implementation.
     * @param parameters List of parameter read-only.
     * @param pluginTranslator Logic to translate parameter changes to plugin calls.
     * @param enabled Initial operational state of the plugin.
     * @return A shared pointer to the newly created context.
     */
    static std::shared_ptr<PluginContext> create(
        const std::shared_ptr<IPlugin>& plugin,
        const std::vector<std::shared_ptr<ParameterInfo>>& parameters,
        const std::shared_ptr<ParameterToPluginTranslator>& pluginTranslator,
        bool enabled = true
    );

    /**
     * @brief Creates a PluginContext for a Plugin that does not have parameters.
     * @param plugin The concrete plugin implementation.
     * @param enabled Initial operational state of the plugin.
     * @return A shared pointer to the newly created context.
     */
    static std::shared_ptr<PluginContext> create(
        const std::shared_ptr<IPlugin>& plugin,
        bool enabled = true
    );


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------
    
    /** @brief Returns the underlying plugin read-only interface. */
    const std::shared_ptr<IPlugin>& getPlugin() const;

    /** @brief Returns a read-only view of this context. */
    std::shared_ptr<PluginContextInfo> getPluginContextView() const;

    /** @brief Sets the plugin to an active state. */
    void enablePlugin(bool enable = true);

    /** @brief Sets the plugin to an inactive state. */
    void disablePlugin();
    
    /** @brief Checks if the plugin is currently active. */
    bool isPluginEnabled() const;

    /** @brief Overridden from ParameterOwner to route values through the translator. */
    TaskResult setParameter(
        const std::string& paramId,
        const std::string& categoryId,
        const std::string& value
    ) override;

    /** @brief Overridden from ParameterOwner to route values through the translator. */
    TaskResult setParameter(
        const std::string& paramId,
        const std::string& categoryId,
        int value
    ) override;

    /** @brief Overridden from ParameterOwner to route values through the translator. */
    TaskResult setParameter(
        const std::string& paramId,
        const std::string& categoryId,
        double value
    ) override;

    /** @brief Overridden from ParameterOwner to route values through the translator. */
    TaskResult setParameter(
        const std::string& paramId,
        const std::string& categoryId,
        bool value
    ) override;

    /** @brief Overridden from ParameterOwner to route values through the translator. */
    TaskResult resetParameter(
        const std::string& paramId,
        const std::string& categoryId
    ) override;

private:
    // Private constructor to enforce factory usage.
    PluginContext(
        const std::shared_ptr<IPlugin>& plugin,
        const std::vector<std::shared_ptr<Parameter>>& parameters,
        const std::shared_ptr<ParameterToPluginTranslator>& pluginTranslator,
        bool enabled = true
    );

    // Private constructor to enforce factory usage.
    PluginContext(
        const std::shared_ptr<IPlugin>& plugin,
        const std::vector<std::shared_ptr<ParameterInfo>>& parameters,
        const std::shared_ptr<ParameterToPluginTranslator>& pluginTranslator,
        bool enabled = true
    );

    // Provides the shared pointer of this instance to the base ParameterOwner.
    std::shared_ptr<const ParameterOwner> getSharedParameterOwner() const override;

    // Pushes current parameter values into the plugin via the translator.
    void applyParametersToPlugin();

private:
    // The actual execution logic.
    std::shared_ptr<IPlugin> plugin_;

    // Maps parameters to plugin parameters.
    std::shared_ptr<ParameterToPluginTranslator> pluginTranslator_;

    // Flag indicating if the plugin is active.
    bool enabled_;

private:
    // Concrete implementation of PluginContextInfo and ParameterOwnerInfoImpl used to provide a read-only interface to a PluginContext.
    class PluginContextInfoImpl : public PluginContextInfo, public ParameterOwnerInfoImpl
    {
    public:
        /** @throw std::invalid_argument if pluginContext is nullptr. */
        PluginContextInfoImpl(const std::shared_ptr<const PluginContext>& pluginContext);

    private:
        bool isPluginEnabled() const override;

    private:
        const std::shared_ptr<const PluginContext> pluginContext_;
    };
};
