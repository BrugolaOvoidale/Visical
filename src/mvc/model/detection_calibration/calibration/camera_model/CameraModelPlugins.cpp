#include "CameraModelPlugins.hpp"
#include <evaluator/PluginContext.hpp>

#include "plugins/ReprojectionErrorCheck/ReprojectionErrorCheck.hpp"
#include "plugins/ReprojectionErrorCheck/ReprojectionErrorCheckRegistry.hpp"
#include "plugins/ReprojectionErrorCheck/ReprojectionErrorCheckTranslator.hpp"


const std::unordered_map<std::string_view, std::function<std::shared_ptr<PluginContext>()>>& CameraModelPlugins::factories()
{
    static const std::unordered_map<std::string_view, std::function<std::shared_ptr<PluginContext>()>> pluginFactories
    {
        {
            ReprojectionErrorCheck::ID,
            []()
            {
                return PluginContext::create(
                    ReprojectionErrorCheck::create(),
                    ReprojectionErrorCheckRegistry().getAllParameters(),
                    std::make_shared<ReprojectionErrorCheckTranslator>()
                );
            }
        }
    };

    return pluginFactories;
}
