#pragma once
#include <unordered_map>
#include <string>
#include <functional>
#include <memory>


// Forward declarations
class PluginContext;


/**
 * @brief Static registry for per-board plugin factories.
 *
 * This class acts as a central repository for all available plugins in the system
 * for board-related checks.
 */
class BoardPlugins
{
public:
	/**
	 * @brief Map of unique identifiers to factory functions that create
	 * instances of PluginContext.
	 */
	static const std::unordered_map<std::string_view, std::function<std::shared_ptr<PluginContext>()>>& factories();
};