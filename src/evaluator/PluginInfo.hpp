#pragma once
#include <string>


/**
 * @brief Abstract interface defining the metadata and capabilities of a plugin.
 * 
 * Any new plugin implementation must inherit from this class and provide
 * specific identification.
 */
class PluginInfo
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    PluginInfo() = default;

    virtual ~PluginInfo() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------
    
    // Unique identifier for the plugin
    virtual const std::string& id() const = 0;

    // Human-readable name
    virtual const std::string& name() const = 0;

    // Description of what this plugin does
    virtual const std::string& description() const = 0;

    // Score threshold
    virtual double getThreshold() const = 0;

    // Support debug results
    virtual bool supportsDebugResults() const = 0;
};
