#include "ParameterCommand.hpp"


ParameterCommand::ParameterCommand(
    const std::string& name,
    const std::string& category,
    AccessMode accessMode,
    VisibilityLevel visibilityLevel,
    const std::optional<std::string>& displayName,
    const std::optional<std::string>& unit,
    const std::optional<std::string>& description
) : Parameter(name, category, DataType::COMMAND, accessMode, visibilityLevel, displayName, unit, description)
{
    isInitialized_ = true;
}

ParameterCommand::ParameterCommand(const ParameterCommand& other)
    : Parameter(other)
{
    isInitialized_ = true;
}

///////////////////////////////////////////////////////////////////

std::shared_ptr<ParameterCommand> ParameterCommand::create(
    const std::string& name,
    const std::string& category,
    AccessMode accessMode,
    VisibilityLevel visibilityLevel,
    const std::optional<std::string>& displayName,
    const std::optional<std::string>& unit,
    const std::optional<std::string>& description)
{
    struct Enabler : public ParameterCommand
    {
        Enabler(
            const std::string& name,
            const std::string& category,
            AccessMode accessMode,
            VisibilityLevel visibilityLevel,
            const std::optional<std::string>& displayName,
            const std::optional<std::string>& unit,
            const std::optional<std::string>& description)
            : ParameterCommand(name, category, accessMode, visibilityLevel, displayName, unit, description) {}
	};

    return std::make_shared<Enabler>(
        name,
        category,
        accessMode,
        visibilityLevel,
        displayName,
        unit,
        description
    );
}

std::shared_ptr<ParameterCommand> ParameterCommand::create(const ParameterCommand& other)
{
    struct Enabler : public ParameterCommand
    {
        Enabler(const ParameterCommand& other) : ParameterCommand(other) {}
	};

    return std::make_shared<Enabler>(other);
}

///////////////////////////////////////////////////////////////////

void ParameterCommand::setValueImpl(
    const void* value,
    const std::type_info& type)
{
    // ignore value
}


void ParameterCommand::getValueImpl(
    void* outValue,
    const std::type_info& type) const
{
    throw std::logic_error("Command has no value");
}