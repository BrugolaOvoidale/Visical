#pragma once
#include <optional>
#include "../Parameter.hpp"


/**
 * @brief Command-specific implementation of the Parameter class.
 */
class ParameterCommand : public Parameter
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    ~ParameterCommand() = default;


    //-----------------------------------------------------------------------------
    // Construction from factories
    //-----------------------------------------------------------------------------

    static std::shared_ptr<ParameterCommand> create(
        const std::string& name,
        const std::string& category,
        AccessMode accessMode,
        VisibilityLevel visibilityLevel,
        const std::optional<std::string>& displayName = std::nullopt,
        const std::optional<std::string>& unit = std::nullopt,
        const std::optional<std::string>& description = std::nullopt
    );

    static std::shared_ptr<ParameterCommand> create(const ParameterCommand& other);

private:
    ParameterCommand(
        const std::string& name,
        const std::string& category,
        AccessMode accessMode,
        VisibilityLevel visibilityLevel,
        const std::optional<std::string>& displayName = std::nullopt,
        const std::optional<std::string>& unit = std::nullopt,
        const std::optional<std::string>& description = std::nullopt
    );

    ParameterCommand(const ParameterCommand& other);

    void setValueImpl(
        const void* value,
        const std::type_info& type
    ) override;

    void getValueImpl(
        void* outValue,
        const std::type_info& type
    ) const override;
};