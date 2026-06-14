#include "BoardPlugins.hpp"
#include <evaluator/PluginContext.hpp>

#include "plugins/ContrastCheck/ContrastCheck.hpp"
#include "plugins/ContrastCheck/ContrastCheckRegistry.hpp"
#include "plugins/ContrastCheck/ContrastCheckTranslator.hpp"

#include "plugins/BoardSizeCheck/BoardSizeCheck.hpp"
#include "plugins/BoardSizeCheck/BoardSizeCheckRegistry.hpp"
#include "plugins/BoardSizeCheck/BoardSizeCheckTranslator.hpp"

#include "plugins/FocusCheck/FocusCheck.hpp"
#include "plugins/FocusCheck/FocusCheckRegistry.hpp"
#include "plugins/FocusCheck/FocusCheckTranslator.hpp"

#include "plugins/IlluminationCheck/IlluminationCheck.hpp"
#include "plugins/IlluminationCheck/IlluminationCheckRegistry.hpp"
#include "plugins/IlluminationCheck/IlluminationCheckTranslator.hpp"

#include "plugins/OverexposureCheck/OverexposureCheck.hpp"
#include "plugins/OverexposureCheck/OverexposureCheckRegistry.hpp"
#include "plugins/OverexposureCheck/OverexposureCheckTranslator.hpp"


const std::unordered_map<std::string_view, std::function<std::shared_ptr<PluginContext>()>>& BoardPlugins::factories()
{
    static const std::unordered_map<std::string_view, std::function<std::shared_ptr<PluginContext>()>> pluginFactories
    {
        {
            ContrastCheck::ID,
            []()
            {
                return PluginContext::create(
                    ContrastCheck::create(),
                    ContrastCheckRegistry().getAllParameters(),
                    std::make_shared<ContrastCheckTranslator>()
                );
            }
        },

        {
            BoardSizeCheck::ID,
            []()
            {
                return PluginContext::create(
                    BoardSizeCheck::create(),
                    BoardSizeCheckRegistry().getAllParameters(),
                    std::make_shared<BoardSizeCheckTranslator>()
                );
            }
        },

        {
            FocusCheck::ID,
            []()
            {
                return PluginContext::create(
                    FocusCheck::create(),
                    FocusCheckRegistry().getAllParameters(),
                    std::make_shared<FocusCheckTranslator>()
                );
            }
        },

        {
            IlluminationCheck::ID,
            []()
            {
                return PluginContext::create(
                    IlluminationCheck::create(),
                    IlluminationCheckRegistry().getAllParameters(),
                    std::make_shared<IlluminationCheckTranslator>()
                );
            }
        },

        {
            OverexposureCheck::ID,
            []()
            {
                return PluginContext::create(
                    OverexposureCheck::create(),
                    OverexposureCheckRegistry().getAllParameters(),
                    std::make_shared<OverexposureCheckTranslator>()
                );
            }
        }
    };

    return pluginFactories;
}
