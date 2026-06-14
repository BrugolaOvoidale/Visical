#include "BoardSequencePlugins.hpp"
#include <evaluator/PluginContext.hpp>

#include "plugins/FOVCoverageCheck/FOVCoverageCheck.hpp"
#include "plugins/FOVCoverageCheck/FOVCoverageCheckRegistry.hpp"
#include "plugins/FOVCoverageCheck/FOVCoverageCheckTranslator.hpp"

#include "plugins/ImagesCountCheck/ImagesCountCheck.hpp"
#include "plugins/ImagesCountCheck/ImagesCountCheckRegistry.hpp"
#include "plugins/ImagesCountCheck/ImagesCountCheckTranslator.hpp"

#include "plugins/ImagesSizeCheck/ImagesSizeCheck.hpp"

#include "plugins/TiltCheck/TiltCheck.hpp"
#include "plugins/TiltCheck/TiltCheckRegistry.hpp"
#include "plugins/TiltCheck/TiltCheckTranslator.hpp"


const std::unordered_map<std::string_view, std::function<std::shared_ptr<PluginContext>()>>& BoardSequencePlugins::factories()
{
    static const std::unordered_map<std::string_view, std::function<std::shared_ptr<PluginContext>()>> pluginFactories
    {
        {
            FOVCoverageCheck::ID,
            []()
            {
                return PluginContext::create(
                    FOVCoverageCheck::create(),
                    FOVCoverageCheckRegistry().getAllParameters(),
                    std::make_shared<FOVCoverageCheckTranslator>()
                );
            }
        },

        {
            ImagesCountCheck::ID,
            []()
            {
                return PluginContext::create(
                    ImagesCountCheck::create(),
                    ImagesCountCheckRegistry().getAllParameters(),
                    std::make_shared<ImagesCountCheckTranslator>()
                );
            }
        },

        {
            ImagesSizeCheck::ID,
            []()
            {
                return PluginContext::create(
                    ImagesSizeCheck::create()
                );
            }
        },

        {
            TiltCheck::ID,
            []()
            {
                return PluginContext::create(
                    TiltCheck::create(),
                    TiltCheckRegistry().getAllParameters(),
                    std::make_shared<TiltCheckTranslator>()
                );
            }
        }
    };

    return pluginFactories;
}
