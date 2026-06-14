#include "BoardEvaluationEngine.hpp"
#include <UtilityFunctions.hpp>
#include <evaluator/IEvaluator.hpp>
#include <evaluator/PluginContext.hpp>
#include <evaluator/IPlugin.hpp>


BoardEvaluationEngine::BoardEvaluationEngine(
    std::shared_ptr<IEvaluator> singleBoardEvaluator,
    std::shared_ptr<IEvaluator> boardSequenceEvaluator)
{
    if (!singleBoardEvaluator)
    {
        throw std::invalid_argument("singleBoardEvaluator is nullptr");
    }

    if (!boardSequenceEvaluator)
    {
        throw std::invalid_argument("boardSequenceEvaluator is nullptr");
    }

	boardEvaluator_ = std::move(singleBoardEvaluator);
	sequenceEvaluator_ = std::move(boardSequenceEvaluator);
}

///////////////////////////////////////////////////////////////////////////////

TaskResult BoardEvaluationEngine::setPluginParameter(
    const std::string& pluginId,
    const std::string& paramId,
    const std::string& categoryId,
    const std::variant<std::monostate, std::string, int, double, bool>& value)
{
    if (boardEvaluator_->hasPlugin(pluginId))
    {
        TaskResult setRes = boardEvaluator_->setPluginParameter(pluginId, paramId, categoryId, value);

        if (setRes.isSuccess())
            reEvaluateAllBoards(pluginId);

        return setRes;
    }
    else if (sequenceEvaluator_->hasPlugin(pluginId))
    {
        TaskResult setRes = sequenceEvaluator_->setPluginParameter(pluginId, paramId, categoryId, value);

        if (setRes.isSuccess())
            reEvaluateSequence(pluginId);

        return setRes;
    }

    return { false, { "No plugin with id " + pluginId, Log::Level::LVL_ERROR } };
}

TaskResult BoardEvaluationEngine::resetPluginParameter(
    const std::string& pluginId,
    const std::string& paramId,
    const std::string& categoryId)
{
    if (boardEvaluator_->hasPlugin(pluginId))
    {
        TaskResult resetRes = boardEvaluator_->resetPluginParameter(pluginId, paramId, categoryId);

        if (resetRes.isSuccess())
            reEvaluateAllBoards(pluginId);

        return resetRes;
    }
    else if (sequenceEvaluator_->hasPlugin(pluginId))
    {
        TaskResult resetRes = sequenceEvaluator_->resetPluginParameter(pluginId, paramId, categoryId);

        if (resetRes.isSuccess())
            reEvaluateSequence(pluginId);

        return resetRes;
    }

    return { false, { "No plugin with id " + pluginId, Log::Level::LVL_ERROR } };
}

TaskResult BoardEvaluationEngine::setPluginThreshold(
    const std::string& pluginId,
    double threshold)
{
    if (boardEvaluator_->hasPlugin(pluginId))
    {
        double oldValue = boardEvaluator_->setPluginThreshold(pluginId, threshold).value();

        if (oldValue != threshold)
            reEvaluateAllBoards(pluginId);

        return true;
    }
    else if (sequenceEvaluator_->hasPlugin(pluginId))
    {
        double oldValue = sequenceEvaluator_->setPluginThreshold(pluginId, threshold).value();

        if (oldValue != threshold)
            reEvaluateSequence(pluginId);

        return true;
    }

    return { false, { "No plugin with id " + pluginId, Log::Level::LVL_ERROR } };
}

std::shared_ptr<PluginContextInfo> BoardEvaluationEngine::getPluginContext(const std::string& pluginId) const
{
    std::shared_ptr<PluginContextInfo> pluginCtx = boardEvaluator_->getRegisteredPlugin(pluginId);

    if (!pluginCtx)
        pluginCtx = sequenceEvaluator_->getRegisteredPlugin(pluginId);

    return pluginCtx;
}

TaskResult BoardEvaluationEngine::loadBoardPlugins(const std::vector<BoardEvaluationEngine::PluginDTO>& config)
{
    return doLoadPlugins(
        config,
        boardEvaluator_,
        getBoardPluginsFactory()
    );
}

std::vector<std::shared_ptr<PluginContextInfo>> BoardEvaluationEngine::getBoardPlugins() const
{
    return boardEvaluator_->getAllRegisteredPlugins();
}

TaskResult BoardEvaluationEngine::registerBoardPlugin(const std::string& pluginId)
{
    const auto& factory = getBoardPluginsFactory();

    auto factoryIt = factory.find(pluginId);
    if (factoryIt == factory.end())
    {
        return { false, { "Plugin " + pluginId + " does not exists", Log::Level::LVL_ERROR } };
    }

    bool success = boardEvaluator_->registerPlugin(factoryIt->second());

    if (!success)
    {
        return { false, { "Plugin " + pluginId + " has not same T as evaluator", Log::Level::LVL_ERROR } };
    }

    reEvaluateAllBoards();

    return true;
}

void BoardEvaluationEngine::unregisterBoardPlugin(const std::string& pluginId)
{
    if (!boardEvaluator_->hasPlugin(pluginId))
        return;

    boardEvaluator_->unregisterPlugin(pluginId);

    reEvaluateAllBoards();
}

std::vector<std::shared_ptr<PluginContextInfo>> BoardEvaluationEngine::getAvailableBoardPlugins() const
{
    const auto& factory = getBoardPluginsFactory();

	std::vector<std::shared_ptr<PluginContextInfo>> availablePlugins;
	availablePlugins.reserve(factory.size());

    for (const auto& [_, f] : factory)
    {
		availablePlugins.push_back(f()->getPluginContextView());
    }

    return availablePlugins;
}

void BoardEvaluationEngine::setGlobalBoardPluginsThreshold(double threshold)
{
    boardEvaluator_->setAllPluginsThreshold(threshold);

    reEvaluateAllBoards();
}

TaskResult BoardEvaluationEngine::loadSequencePlugins(const std::vector<BoardEvaluationEngine::PluginDTO>& config)
{
    return doLoadPlugins(
        config,
        sequenceEvaluator_,
        getSequencePluginsFactory()
    );
}

std::vector<std::shared_ptr<PluginContextInfo>> BoardEvaluationEngine::getSequencePlugins() const
{
    return sequenceEvaluator_->getAllRegisteredPlugins();
}

TaskResult BoardEvaluationEngine::registerSequencePlugin(const std::string& pluginId)
{
    const auto& factory = getSequencePluginsFactory();

    auto factoryIt = factory.find(pluginId);
    if (factoryIt == factory.end())
    {
        return { false, { "Plugin " + pluginId + " does not exists", Log::Level::LVL_ERROR } };
    }

    bool success = sequenceEvaluator_->registerPlugin(factoryIt->second());

    if (!success)
    {
        return { false, { "Plugin " + pluginId + " has not same T as evaluator", Log::Level::LVL_ERROR } };
    }

    reEvaluateSequence();

    return true;
}

void BoardEvaluationEngine::unregisterSequencePlugin(const std::string& pluginId)
{
    if (!boardEvaluator_->hasPlugin(pluginId))
        return;

    sequenceEvaluator_->unregisterPlugin(pluginId);

    reEvaluateSequence();
}

std::vector<std::shared_ptr<PluginContextInfo>> BoardEvaluationEngine::getAvailableSequencePlugins() const
{
    const auto& factory = getSequencePluginsFactory();

    std::vector<std::shared_ptr<PluginContextInfo>> availablePlugins;
    availablePlugins.reserve(factory.size());

    for (const auto& [_, f] : factory)
    {
        availablePlugins.push_back(f()->getPluginContextView());
    }

    return availablePlugins;
}

void BoardEvaluationEngine::setGlobalSequencePluginsThreshold(double threshold)
{
    sequenceEvaluator_->setAllPluginsThreshold(threshold);

    reEvaluateSequence();
}

TaskResult BoardEvaluationEngine::enablePlugin(
    const std::string& pluginId,
    bool enable)
{
    if (boardEvaluator_->hasPlugin(pluginId))
    {
        bool oldValue = boardEvaluator_->enablePlugin(pluginId, enable).value();

        if (oldValue != enable)
            reEvaluateAllBoards(pluginId, enable);

        return true;
    }
    else if (sequenceEvaluator_->hasPlugin(pluginId))
    {
        bool oldValue = sequenceEvaluator_->enablePlugin(pluginId, enable).value();

        if (oldValue != enable)
            reEvaluateSequence(pluginId, enable);

        return true;
    }

    return { false, { "No plugin with id " + pluginId, Log::Level::LVL_ERROR } };
}

///////////////////////////////////////////////////////////////////////////////

std::shared_ptr<IEvaluator> BoardEvaluationEngine::singleBoardEvaluator() const
{
    return boardEvaluator_;
}

std::shared_ptr<IEvaluator> BoardEvaluationEngine::sequenceEvaluator() const
{
    return sequenceEvaluator_;
}

///////////////////////////////////////////////////////////////////////////////

TaskResult BoardEvaluationEngine::doLoadPlugins(
    const std::vector<BoardEvaluationEngine::PluginDTO>& config,
    const std::shared_ptr<IEvaluator>& evaluator,
    const std::unordered_map<std::string_view, std::function<std::shared_ptr<PluginContext>()>>& factory)
{
    std::vector<Log> exitLogs;

    evaluator->unregisterAllPlugins();

    for (const auto& c : config)
    {
        const std::string& pluginId = c.id;

        auto factoryIt = factory.find(pluginId);
        if (factoryIt == factory.end())
        {
            exitLogs.push_back("Plugin " + pluginId + " does not exists");
        }

        std::shared_ptr<PluginContext> pluginCtx = factoryIt->second();
        pluginCtx->enablePlugin(c.enabled);
        pluginCtx->getPlugin()->setThreshold(c.threshold);

        bool success = evaluator->registerPlugin(pluginCtx);

        if (success)
        {
            for (const auto& p : c.parameters)
            {
                TaskResult setRes = configurePluginParameter(pluginId, p.paramId, p.categoryId, p.value);

                UtilityFunctions::moveInto(setRes.takeLogs(), exitLogs);
            }
        }
    }

    return { true , std::move(exitLogs) };
}

TaskResult BoardEvaluationEngine::configurePluginParameter(
    const std::string& pluginId,
    const std::string& paramId,
    const std::string& categoryId,
    const std::variant<std::monostate, std::string, int, double, bool>& value)
{
    if (boardEvaluator_->hasPlugin(pluginId))
    {
        TaskResult setRes = boardEvaluator_->setPluginParameter(pluginId, paramId, categoryId, value);

        return setRes;
    }
    else if (sequenceEvaluator_->hasPlugin(pluginId))
    {
        TaskResult setRes = sequenceEvaluator_->setPluginParameter(pluginId, paramId, categoryId, value);

        return setRes;
    }

    return { false, { "No plugin with id " + pluginId, Log::Level::LVL_ERROR } };
}

