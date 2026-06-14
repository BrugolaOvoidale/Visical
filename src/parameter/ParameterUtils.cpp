#include "ParameterUtils.hpp"
#include "command/ParameterCommand.hpp"
#include "numeric/ParameterNumeric.hpp"
#include "enum/ParameterEnum.hpp"
#include "bool/ParameterBool.hpp"
#include "string/ParameterString.hpp"


std::shared_ptr<Parameter> ParameterUtils::clone(
    const std::shared_ptr<Parameter>& param,
    const std::optional<AccessMode>& overrideAccessMode)
{
    const AccessMode accessMode = overrideAccessMode.value_or(param->accessMode());

    std::shared_ptr<Parameter> clone;
    switch (param->dataType())
    {
    case DataType::BOOLEAN:
    {
        std::shared_ptr<ParameterBool> boolParam = std::dynamic_pointer_cast<ParameterBool>(param);

        clone = ParameterBool::create(
            boolParam->name(),
            boolParam->category(),
            accessMode,
            boolParam->visibilityLevel(),
            boolParam->getValue(),
            boolParam->displayName(),
            boolParam->unit(),
            boolParam->description()
        );

        break;
    }

    case DataType::COMMAND:
    {
        clone = ParameterCommand::create(
            param->name(),
            param->category(),
            accessMode,
            param->visibilityLevel(),
            param->displayName(),
            param->unit(),
            param->description()
        );

        break;
    }

    case DataType::DOUBLE:
    {
        std::shared_ptr<ParameterDouble> doubleParam = std::dynamic_pointer_cast<ParameterDouble>(param);

        clone = ParameterDouble::create(
            doubleParam->name(),
            doubleParam->category(),
            accessMode,
            doubleParam->visibilityLevel(),
            doubleParam->getValue(),
            doubleParam->getMin(),
            doubleParam->getMax(),
            doubleParam->getIncrement(),
            doubleParam->displayName(),
            doubleParam->unit(),
            doubleParam->description()
        );

        break;
    }

    case DataType::ENUMERATION:
    {
        std::shared_ptr<ParameterEnum> enumParam = std::dynamic_pointer_cast<ParameterEnum>(param);

        clone = ParameterEnum::create(
            enumParam->name(),
            enumParam->category(),
            accessMode,
            enumParam->visibilityLevel(),
            enumParam->getPossibleIntValues(),
            enumParam->getValue(),
            enumParam->displayName(),
            enumParam->unit(),
            enumParam->description()
        );

        break;
    }

    case DataType::INTEGER:
    {
        std::shared_ptr<ParameterInt> intParam = std::dynamic_pointer_cast<ParameterInt>(param);

        clone = ParameterInt::create(
            intParam->name(),
            intParam->category(),
            accessMode,
            intParam->visibilityLevel(),
            intParam->getValue(),
            intParam->getMin(),
            intParam->getMax(),
            intParam->getIncrement(),
            intParam->displayName(),
            intParam->unit(),
            intParam->description()
        );

        break;
    }

    case DataType::STRING:
    {
        std::shared_ptr<ParameterString> stringParam = std::dynamic_pointer_cast<ParameterString>(param);

        clone = ParameterString::create(
            stringParam->name(),
            stringParam->category(),
            accessMode,
            stringParam->visibilityLevel(),
            stringParam->getValue(),
            stringParam->displayName(),
            stringParam->unit(),
            stringParam->description()
        );

        break;
    }

    default:
        break;
    }

    return clone;
}

std::shared_ptr<Parameter> ParameterUtils::cloneToParameter(
    const std::shared_ptr<ParameterInfo>& infoParam,
    const std::optional<AccessMode>& overrideAccessMode)
{
    const AccessMode accessMode = overrideAccessMode.value_or(infoParam->accessMode());

    std::shared_ptr<Parameter> param;
    switch (infoParam->dataType())
    {
    case DataType::BOOLEAN:
    {
        std::shared_ptr<ParameterBoolInfo> boolParam = std::dynamic_pointer_cast<ParameterBoolInfo>(infoParam);

        param = ParameterBool::create(
            boolParam->name(),
            boolParam->category(),
            accessMode,
            boolParam->visibilityLevel(),
            boolParam->getValue(),
            boolParam->displayName(),
            boolParam->unit(),
            boolParam->description()
        );

        break;
    }

    case DataType::COMMAND:
    {
        param = ParameterCommand::create(
            infoParam->name(),
            infoParam->category(),
            accessMode,
            infoParam->visibilityLevel(),
            infoParam->displayName(),
            infoParam->unit(),
            infoParam->description()
        );

        break;
    }

    case DataType::DOUBLE:
    {
        std::shared_ptr<ParameterDoubleInfo> doubleParam = std::dynamic_pointer_cast<ParameterDoubleInfo>(infoParam);

        param = ParameterDouble::create(
            doubleParam->name(),
            doubleParam->category(),
            accessMode,
            doubleParam->visibilityLevel(),
            doubleParam->getValue(),
            doubleParam->getMin(),
            doubleParam->getMax(),
            doubleParam->getIncrement(),
            doubleParam->displayName(),
            doubleParam->unit(),
            doubleParam->description()
        );

        break;
    }

    case DataType::ENUMERATION:
    {
        std::shared_ptr<ParameterEnumInfo> enumParam = std::dynamic_pointer_cast<ParameterEnumInfo>(infoParam);

        param = ParameterEnum::create(
            enumParam->name(),
            enumParam->category(),
            accessMode,
            enumParam->visibilityLevel(),
            enumParam->getPossibleIntValues(),
            enumParam->getValue(),
            enumParam->displayName(),
            enumParam->unit(),
            enumParam->description()
        );

        break;
    }

    case DataType::INTEGER:
    {
        std::shared_ptr<ParameterIntInfo> intParam = std::dynamic_pointer_cast<ParameterIntInfo>(infoParam);

        param = ParameterInt::create(
            intParam->name(),
            intParam->category(),
            accessMode,
            intParam->visibilityLevel(),
            intParam->getValue(),
            intParam->getMin(),
            intParam->getMax(),
            intParam->getIncrement(),
            intParam->displayName(),
            intParam->unit(),
            intParam->description()
        );

        break;
    }

    case DataType::STRING:
    {
        std::shared_ptr<ParameterStringInfo> stringParam = std::dynamic_pointer_cast<ParameterStringInfo>(infoParam);

        param = ParameterString::create(
            stringParam->name(),
            stringParam->category(),
            accessMode,
            stringParam->visibilityLevel(),
            stringParam->getValue(),
            stringParam->displayName(),
            stringParam->unit(),
            stringParam->description()
        );

        break;
    }

    default:
        break;
    }

    return param;
}
