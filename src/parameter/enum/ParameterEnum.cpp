#include "ParameterEnum.hpp"


ParameterEnum::ParameterEnum(
    const std::string& name,
    const std::string& category,
    AccessMode accessMode,
    VisibilityLevel visibilityLevel,
    std::unordered_map<int, std::string> possibleValues,
    int value,
    const std::optional<std::string>& displayName,
    const std::optional<std::string>& unit,
    const std::optional<std::string>& description)
    : Parameter(name, category, DataType::ENUMERATION, accessMode, visibilityLevel, displayName, unit, description),
    possibleIntValues_(possibleValues)
{
    if (possibleValues.empty())
    {
        throw std::invalid_argument("possibleValues is empty");
    }

    setValue(value);

    for (const auto& [key, value] : possibleValues)
    {
        possibleStrValues_.emplace(value, key);
    }

    isInitialized_ = true;
}

ParameterEnum::ParameterEnum(
    const std::string& name,
    const std::string& category,
    AccessMode accessMode,
    VisibilityLevel visibilityLevel,
    std::unordered_map<int, std::string> possibleValues,
    const std::string& value,
    const std::optional<std::string>& displayName,
    const std::optional<std::string>& unit,
    const std::optional<std::string>& description)
    : Parameter(name, category, DataType::ENUMERATION, accessMode, visibilityLevel, displayName, unit, description),
    possibleIntValues_(possibleValues)
{
    if (possibleValues.empty())
    {
        throw std::invalid_argument("possibleValues is empty");
    }

    for (const auto& [key, value] : possibleValues)
    {
        possibleStrValues_.emplace(value, key);
    }

    setValue(value);

    isInitialized_ = true;
}

ParameterEnum::ParameterEnum(const ParameterEnum& other)
    : Parameter(other),
    possibleIntValues_(other.getPossibleIntValues()), possibleStrValues_(other.getPossibleStrValues()), value_(other.getValue())
{
    isInitialized_ = true;
}

/////////////////////////////////////////////////////////////////////////

std::shared_ptr<ParameterEnum> ParameterEnum::create(
    const std::string& name,
    const std::string& category,
    AccessMode accessMode,
    VisibilityLevel visibilityLevel,
    std::unordered_map<int, std::string> possibleValues,
    int value,
    const std::optional<std::string>& displayName,
    const std::optional<std::string>& unit,
    const std::optional<std::string>& description)
{
    struct Enabler : public ParameterEnum
    {
        Enabler(
            const std::string& name,
            const std::string& category,
            AccessMode accessMode,
            VisibilityLevel visibilityLevel,
            std::unordered_map<int, std::string> possibleValues,
            int value,
            const std::optional<std::string>& displayName,
            const std::optional<std::string>& unit,
            const std::optional<std::string>& description)
            : ParameterEnum(name, category, accessMode, visibilityLevel, possibleValues, value, displayName, unit, description)
        {
        }
	};

    return std::make_shared<Enabler>(
        name,
        category,
        accessMode,
        visibilityLevel,
        possibleValues,
        value,
        displayName,
        unit,
        description
    );
}

std::shared_ptr<ParameterEnum> ParameterEnum::create(
    const std::string& name,
    const std::string& category,
    AccessMode accessMode,
    VisibilityLevel visibilityLevel,
    std::unordered_map<int, std::string> possibleValues,
    const std::string& value,
    const std::optional<std::string>& displayName,
    const std::optional<std::string>& unit,
    const std::optional<std::string>& description)
{
    struct Enabler : public ParameterEnum
    {
        Enabler(
            const std::string& name,
            const std::string& category,
            AccessMode accessMode,
            VisibilityLevel visibilityLevel,
            std::unordered_map<int, std::string> possibleValues,
            const std::string& value,
            const std::optional<std::string>& displayName,
            const std::optional<std::string>& unit,
            const std::optional<std::string>& description)
            : ParameterEnum(name, category, accessMode, visibilityLevel, possibleValues, value, displayName, unit, description)
        {
        }
	};

    return std::make_shared<Enabler>(
        name,
        category,
        accessMode,
        visibilityLevel,
        possibleValues,
        value,
        displayName,
        unit,
        description
    );
}

std::shared_ptr<ParameterEnum> ParameterEnum::create(const ParameterEnum& other)
{
    struct Enabler : public ParameterEnum
    {
        Enabler(const ParameterEnum& other) : ParameterEnum(other) {}
	};

    return std::make_shared<Enabler>(other);
}

/////////////////////////////////////////////////////////////////////////

void ParameterEnum::setValue(int newValue)
{
    if (isInitialized_ && !isWritable())
    {
        throw std::logic_error("Parameter is not writable");
    }

    auto it = possibleIntValues_.find(newValue);
    if (it == possibleIntValues_.end())
    {
        throw std::logic_error("new value is not a possible value");
    }

    std::unique_lock lock(mutex_);
    value_ = newValue;
}

void ParameterEnum::setValue(const std::string& newValue)
{
    if (isInitialized_ && !isWritable())
    {
        throw std::logic_error("Parameter is not writable");
    }

    auto it = possibleStrValues_.find(newValue);
    if (it == possibleStrValues_.end())
    {
        throw std::logic_error("new value is not a possible value");
    }

    std::unique_lock lock(mutex_);
    value_ = it->second;
}

int ParameterEnum::getValue() const
{
    std::shared_lock lock(mutex_);
    return value_;
};

const std::string& ParameterEnum::getStrValue() const
{
    std::shared_lock lock(mutex_);
    return possibleIntValues_.at(value_);
};

std::shared_ptr<ParameterInfo> ParameterEnum::getParameterView() const
{
    return std::static_pointer_cast<ParameterInfo>(
        std::make_shared<ParameterEnumInfoImpl>(shared_from_this())
    );
}

std::optional<int> ParameterEnum::getStrValueAsInt(const std::string& value) const
{
    auto it = possibleStrValues_.find(value);
    if (it == possibleStrValues_.end()) return std::nullopt;

    return it->second;
}

std::optional<std::string> ParameterEnum::getIntValueAsStr(int value) const
{
    auto it = possibleIntValues_.find(value);
    if (it == possibleIntValues_.end()) return std::nullopt;

    return it->second;
}

const std::unordered_map<int, std::string>& ParameterEnum::getPossibleIntValues() const
{
    return possibleIntValues_;
}

const std::unordered_map<std::string, int>& ParameterEnum::getPossibleStrValues() const
{
    return possibleStrValues_;
}

/////////////////////////////////////////////////////////////////////////

void ParameterEnum::setValueImpl(
    const void* value,
    const std::type_info& type)
{
    if (type == typeid(int))
    {
        setValue(
            *static_cast<const int*>(value)
        );
    }
    else if (type == typeid(std::string))
    {
        setValue(
            *static_cast<const std::string*>(value)
        );
    }
    else
    {
        throw std::bad_cast();
    }
}


void ParameterEnum::getValueImpl(
    void* outValue,
    const std::type_info& type) const
{
    if (type == typeid(int))
    {
        *static_cast<int*>(outValue) = getValue();
    }
    else if (type == typeid(std::string))
    {
        *static_cast<std::string*>(outValue) = getStrValue();
    }
    else
    {
        throw std::bad_cast();
    }
}


/////////////////////////////////////////////////////////////////////////


ParameterEnum::ParameterEnumInfoImpl::ParameterEnumInfoImpl(const std::shared_ptr<const Parameter>& param)
    : ParameterInfoImpl(param), enumParam_(std::static_pointer_cast<const ParameterEnum>(param))
{
}

/////////////////////////////////////////////////////////////////////////

int ParameterEnum::ParameterEnumInfoImpl::getValue() const
{
    return enumParam_->getValue();
}

const std::string& ParameterEnum::ParameterEnumInfoImpl::getStrValue() const
{
    return enumParam_->getStrValue();
}

const std::unordered_map<int, std::string>& ParameterEnum::ParameterEnumInfoImpl::getPossibleIntValues() const
{
    return enumParam_->getPossibleIntValues();
}

const std::unordered_map<std::string, int>& ParameterEnum::ParameterEnumInfoImpl::getPossibleStrValues() const
{
    return enumParam_->getPossibleStrValues();
}