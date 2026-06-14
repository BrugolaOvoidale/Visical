#include "CalibratedBoardPlugins.hpp"
#include <evaluator/PluginContext.hpp>

#include "plugins/MaxErrorCheck/MaxErrorCheck.hpp"
#include "plugins/MaxErrorCheck/MaxErrorCheckRegistry.hpp"
#include "plugins/MaxErrorCheck/MaxErrorCheckTranslator.hpp"

#include "plugins/RMSErrorCheck/RMSErrorCheck.hpp"
#include "plugins/RMSErrorCheck/RMSErrorCheckRegistry.hpp"
#include "plugins/RMSErrorCheck/RMSErrorCheckTranslator.hpp"


const std::unordered_map<std::string_view, std::function<std::shared_ptr<PluginContext>()>>& CalibratedBoardPlugins::factories()
{
    static const std::unordered_map<std::string_view, std::function<std::shared_ptr<PluginContext>()>> pluginFactories
    {
        {
            MaxErrorCheck::ID,
            []()
            {
                return PluginContext::create(
                    MaxErrorCheck::create(),
                    MaxErrorCheckRegistry().getAllParameters(),
                    std::make_shared<MaxErrorCheckTranslator>()
                );
            }
        },

        {
            RMSErrorCheck::ID,
            []()
            {
                return PluginContext::create(
                    RMSErrorCheck::create(),
                    RMSErrorCheckRegistry().getAllParameters(),
                    std::make_shared<RMSErrorCheckTranslator>()
                );
            }
        }
    };

    return pluginFactories;
}
