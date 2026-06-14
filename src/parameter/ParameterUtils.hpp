#pragma once
#include <memory>
#include <optional>
#include "ParameterDefs.hpp"
#include "Parameter.hpp"


/**
 * @brief Static utility collection for Parameter manipulation and type conversion.
 * This class provides helper functions for deep-cloning parameters and
 * performing specialized template-based casting between Parameter types.
 */
class ParameterUtils
{
public:
    /**
     * @brief Creates a deep copy of an existing parameter.
     * @param param The source parameter to clone.
     * @param overrideAccessMode Optional new access mode for the cloned parameter.
     * If nullopt, the source's access mode is preserved.
     * @return A new std::shared_ptr<Parameter> (shared_ptr) owning the cloned object.
     */
    static std::shared_ptr<Parameter> clone(
        const std::shared_ptr<Parameter>& param,
        const std::optional<AccessMode>& overrideAccessMode = std::nullopt);

    /**
     * @brief Instantiates a new Parameter based on a ParameterInfo definition.
     * @param infoParam The metadata/info object used as a template for construction.
     * @param overrideAccessMode Optional access mode to apply to the new instance.
     * @return A std::shared_ptr<Parameter> initialized with the values/metadata from infoParam.
     */
    static std::shared_ptr<Parameter> cloneToParameter(
        const std::shared_ptr<ParameterInfo>& infoParam,
        const std::optional<AccessMode>& overrideAccessMode = std::nullopt
    );
};
