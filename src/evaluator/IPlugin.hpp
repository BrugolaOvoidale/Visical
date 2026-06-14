#pragma once
#include <memory>
#include "PluginInfo.hpp"


// Base class for all evaluation plugins
class IPlugin : public std::enable_shared_from_this<IPlugin>
{
public:
    //-----------------------------------------------------------------------------
    // Constants
    //-----------------------------------------------------------------------------

    /** @brief Default score threshold (percentage). */
    static constexpr double DEFAULT_THRESHOLD = 70.0;

    /** @brief Default critical threshold, defined as a fraction of the standard threshold. */
    static constexpr double DEFAULT_CRITICAL_THR = 0.5;


    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    virtual ~IPlugin() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------
    
    /** @brief Returns the unique identifier of the plugin. */
    const std::string& id() const;

    /** @brief Returns the human-readable name of the plugin. */
    const std::string& name() const;

    /** @brief Returns a description of what this plugin does. */
    const std::string& description() const;

    /** @brief Returns the current score threshold. */
    double getThreshold() const;

    /** @brief Updates the score threshold. */
    void setThreshold(double thr);

    /**
     * @brief Determines if the plugin can provide detailed diagnostic/debug results.
     * @return True if debug results are supported.
     */
    virtual bool supportsDebugResults() const = 0;

    /**
     * @brief Creates a read-only view of the plugin metadata.
     * @return A shared_ptr to a PluginInfo object.
     */
    std::shared_ptr<PluginInfo> getPluginView() const;

protected:
    // Protected constructor to ensure instantiation via factory methods or subclasses.
    IPlugin(
        const std::string& id,
        const std::string& name,
        const std::string& description,
        double threshold = DEFAULT_THRESHOLD
    );

private:
    // Unique internal Id.
    std::string id_;

    // Display name
    std::string name_;

    // Plugin description
    std::string description_;
    
    // Score threshold value
    double threshold_;

private:
    // Concrete implementation of PluginInfo used to provide a read-only interface to a Plugin.
    class PluginInfoImpl : public PluginInfo
    {
    public:
        /** @throw std::invalid_argument if plugin is nullptr. */
        PluginInfoImpl(const std::shared_ptr<const IPlugin>& plugin);

    private:
        const std::string& id() const override;

        const std::string& name() const override;

        const std::string& description() const override;

        double getThreshold() const override;

        bool supportsDebugResults() const override;

    private:
        const std::shared_ptr<const IPlugin> plugin_;
    };
};
