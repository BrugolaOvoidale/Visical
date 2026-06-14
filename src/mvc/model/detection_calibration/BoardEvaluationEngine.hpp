#pragma once
#include <functional>
#include <parameter/ParameterOwner.hpp>


// Forward declarations
class PluginContext;
class PluginContextInfo;
class IEvaluator;


/**
 * @brief Orchestrates the evaluation of boards and sequences using a plugin-based architecture.
 *
 * This abstract base class manages the lifecycle, configuration, and execution of plugins
 * divided into two scopes: Single Board and Board Sequence.
 */
class BoardEvaluationEngine
{
public:
	//-----------------------------------------------------------------------------
	// Structs
	//-----------------------------------------------------------------------------

	/** @brief DTO for a plugin configuration. */
	struct PluginDTO
	{
		std::string id;
		double threshold;
		bool enabled;
		std::vector<ParameterOwner::DTO> parameters;
	};


	//-----------------------------------------------------------------------------
	// Constructor/Destructor
	//-----------------------------------------------------------------------------

	virtual ~BoardEvaluationEngine() = default;


	//-----------------------------------------------------------------------------
	// Class operators
	//-----------------------------------------------------------------------------

	/**
	 * @brief Updates a specific plugin parameter and triggers a re-evaluation.
	 * @param pluginId Target plugin Id.
	 * @param paramId Parameter identifier.
	 * @param categoryId Parameter category.
	 * @param value The new value.
	 * @return TaskResult indicating if the update succeeded.
	 */
	TaskResult setPluginParameter(
		const std::string& pluginId,
		const std::string& paramId,
		const std::string& categoryId,
		const std::variant<std::monostate, std::string, int, double, bool>& value
	);

	/** @brief Resets a parameter to its default value and triggers re-evaluation. */
	TaskResult resetPluginParameter(
		const std::string& pluginId,
		const std::string& paramId,
		const std::string& categoryId
	);

	/** @brief Updates the threshold for a plugin and triggers re-evaluation. */
	TaskResult setPluginThreshold(
		const std::string& pluginId,
		double threshold
	);

	/** @brief Retrieves read-only interface of a specific plugin. */
	std::shared_ptr<PluginContextInfo> getPluginContext(const std::string& pluginId) const;

	/** @brief Loads and initializes board-level plugins from a configuration vector. */
	TaskResult loadBoardPlugins(const std::vector<PluginDTO>& config);

	/** @brief Returns list of currently loaded board plugins. */
	std::vector<std::shared_ptr<PluginContextInfo>> getBoardPlugins() const;

	/**
	 * @brief Instantiates a new board plugin by Id and triggers re-evaluation.
	 * @param pluginId Target plugin Id.
	 * @return true if it exists in factory, false otherwise.
	 */
	TaskResult registerBoardPlugin(const std::string& pluginId);

	/** @brief Detaches a board plugin and triggers re-evaluation. */
	void unregisterBoardPlugin(const std::string& pluginId);

	/** @brief Lists all board plugins available in the factory but not necessarily loaded. */
	std::vector<std::shared_ptr<PluginContextInfo>> getAvailableBoardPlugins() const;

	/** @brief Sets a uniform threshold across all board-level plugins. */
	void setGlobalBoardPluginsThreshold(double threshold);

	/** @brief Loads and initializes sequence-level plugins from a configuration vector. */
	TaskResult loadSequencePlugins(const std::vector<PluginDTO>& config);

	/** @brief Returns list of currently loaded sequence plugins. */
	std::vector<std::shared_ptr<PluginContextInfo>> getSequencePlugins() const;

	/**
	 * @brief Instantiates a new sequence plugin by Id and triggers re-evaluation.
	 * @param pluginId Target plugin Id.
	 * @return true if it exists in factory, false otherwise.
	 */
	TaskResult registerSequencePlugin(const std::string& pluginId);

	/** @brief Detaches a sequence plugin and triggers re-evaluation. */
	void unregisterSequencePlugin(const std::string& pluginId);

	/** @brief Lists all sequence plugins available in the factory but not necessarily loaded. */
	std::vector<std::shared_ptr<PluginContextInfo>> getAvailableSequencePlugins() const;

	/** @brief Sets a uniform threshold across all sequence-level plugins. */
	void setGlobalSequencePluginsThreshold(double threshold);

	/** @brief Toggles a plugin's active state. */
	TaskResult enablePlugin(
		const std::string& pluginId,
		bool enable
	);

protected:
	/**
	 * @brief Protected constructor to be called by derived specialized engines.
	 * @param singleBoardEvaluator Evaluator instance for individual items.
	 * @param boardSequenceEvaluator Evaluator instance for item streams.
	 */
	BoardEvaluationEngine(
		std::shared_ptr<IEvaluator> singleBoardEvaluator,
		std::shared_ptr<IEvaluator> boardSequenceEvaluator
	);

	std::shared_ptr<IEvaluator> singleBoardEvaluator() const;

	std::shared_ptr<IEvaluator> sequenceEvaluator() const;

	// Re-evaluation methods that derived classes must implement to trigger the appropriate evaluation pipelines when plugins are modified.

	virtual TaskEnqueueResult reEvaluateAllBoards() = 0;

	virtual TaskEnqueueResult reEvaluateAllBoards(
		const std::string& id,
		bool enabled = true
	) = 0;

	virtual TaskEnqueueResult reEvaluateSequence() = 0;

	virtual TaskEnqueueResult reEvaluateSequence(
		const std::string& id,
		bool enabled = true
	) = 0;

private:
	// Provides the mapping from Id to plugin creation logic for Board scope.
	virtual const std::unordered_map<std::string_view, std::function<std::shared_ptr<PluginContext>()>>& getBoardPluginsFactory() const = 0;

	// Provides the mapping from Id to plugin creation logic for Sequence scope.
	virtual const std::unordered_map<std::string_view, std::function<std::shared_ptr<PluginContext>()>>& getSequencePluginsFactory() const = 0;

	// Like "setPluginParameter", but does not trigger a re-evaluation. Used internally when loading plugins from config.
	TaskResult configurePluginParameter(
		const std::string& pluginId,
		const std::string& paramId,
		const std::string& categoryId,
		const std::variant<std::monostate, std::string, int, double, bool>& value
	);

	// common logic for loading plugins into a specific evaluator via a factory.
	TaskResult doLoadPlugins(
		const std::vector<BoardEvaluationEngine::PluginDTO>& config,
		const std::shared_ptr<IEvaluator>& evaluator,
		const std::unordered_map<std::string_view, std::function<std::shared_ptr<PluginContext>()>>& factory
	);

private:
	// Evaluator for single boards.
	std::shared_ptr<IEvaluator> boardEvaluator_;

	// Evaluator for board sequences.
	std::shared_ptr<IEvaluator> sequenceEvaluator_;
};
