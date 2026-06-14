#pragma once
#include <parameter/ParameterOwnerInfo.hpp>


// Forward declarations
class PluginInfo;


/**
 * @brief Provides contextual information for a plugin instance within the parameter system.
 * 
 * This class links a specific plugin's metadata (PluginInfo) with the parameter
 * ownership logic. It is designed as an abstract base class to manage plugin state
 * and lifecycle information.
 */
class PluginContextInfo : public virtual ParameterOwnerInfo
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    /**
     * @brief Constructs a new Plugin Context Info object.
     * @param pluginInfo A shared pointer to the plugin's metadata.
     * @throw std::runtime_error if plugin is nullptr
     */
    PluginContextInfo(const std::shared_ptr<PluginInfo>& pluginInfo);

    virtual ~PluginContextInfo() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------
    
    /**
     * @brief Retrieves the associated plugin metadata.
     * @return A reference to the shared_ptr containing PluginInfo.
     */
    const std::shared_ptr<PluginInfo>& getPluginInfo() const;

    /**
     * @brief Checks if the plugin is currently active.
     * @return true if the plugin is enabled, false otherwise.
     */
    virtual bool isPluginEnabled() const = 0;

private:
    std::shared_ptr<PluginInfo> pluginInfo_;
};
