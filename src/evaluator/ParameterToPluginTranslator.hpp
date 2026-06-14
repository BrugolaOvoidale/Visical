#pragma once
#include <vector>
#include <memory>


// Forward declarations
class IPlugin;
class ParameterInfo;


/**
 * @brief Abstract interface for translating and applying parameters to a plugin.
 * 
 * This class defines the strategy for taking a generic collection of parameters
 * and mapping them to the specific internal state of an IPlugin.
 * It allows the system to remain agnostic of how different plugins ingest data.
 */
class ParameterToPluginTranslator
{
public:
	//-----------------------------------------------------------------------------
	// Constructor/Destructor
	//-----------------------------------------------------------------------------

	ParameterToPluginTranslator() = default;

	virtual ~ParameterToPluginTranslator() = default;


	//-----------------------------------------------------------------------------
	// Class operators
	//-----------------------------------------------------------------------------

	/**
	 * @brief Pure virtual method to apply a set of parameters to a specific plugin.
	 * 
	 * Derived classes must implement this logic to handle type casting and
	 * assignment from the generic ParameterInfo objects to the plugin's API.
	 * 
	 * @param plugin The target plugin instance to update.
	 * @param parameters A list of shared pointers to the parameters to be applied.
	 */
	virtual void applyParametersToPlugin(
		const std::shared_ptr<IPlugin>& plugin,
		const std::vector<std::shared_ptr<ParameterInfo>>& parameters
	) = 0;
};
