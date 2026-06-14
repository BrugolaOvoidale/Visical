#pragma once
#include <unordered_map>
#include <string>
#include <functional>
#include <memory>


// Forward declarations
class PluginContext;


/**
 * @brief Static registry for board sequence plugin factories.
 *
 * This class acts as a central repository for all available plugins in the system
 * for sequence-related checks.
 */
class BoardSequencePlugins
{
public:
	/**
	 * @brief Map of unique identifiers to factory functions that create
	 * instances of PluginContext.
	 */
	static const std::unordered_map<std::string_view, std::function<std::shared_ptr<PluginContext>()>>& factories();
};