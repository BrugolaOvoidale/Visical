#include "ParameterString.hpp"


ParameterString::ParameterString(
    const std::string& name,
    const std::string& category,
    AccessMode accessMode,
    VisibilityLevel visibilityLevel,
    const std::string& value,
    const std::optional<std::string>& displayName,
    const std::optional<std::string>& unit,
    const std::optional<std::string>& description)
    : Parameter(name, category, DataType::STRING, accessMode, visibilityLevel, displayName, unit, description),
    value_(value)
{
    setValue(value);

    isInitialized_ = true;
}

ParameterString::ParameterString(const ParameterString& other)
: Parameter(other),value_(other.getValue())
{
    isInitialized_ = true;
}

///////////////////////////////////////////////////////////////

std::shared_ptr<ParameterString> ParameterString::create(
    const std::string& name,
    const std::string& category,
    AccessMode accessMode,
    VisibilityLevel visibilityLevel,
    const std::string& value,
    const std::optional<std::string>& displayName,
    const std::optional<std::string>& unit,
    const std::optional<std::string>& description)
{
    struct Enabler : public ParameterString
    {
        Enabler(
            const std::string& name,
            const std::string& category,
            AccessMode accessMode,
            VisibilityLevel visibilityLevel,
            const std::string& value,
            const std::optional<std::string>& displayName,
            const std::optional<std::string>& unit,
            const std::optional<std::string>& description)
            : ParameterString(name, category, accessMode, visibilityLevel, value, displayName, unit, description) {}
	};

    return std::make_shared<Enabler>(
        name,
        category,
        accessMode,
        visibilityLevel,
        value,
        displayName,
        unit,
        description
    );
}

std::shared_ptr<ParameterString> ParameterString::create(const ParameterString& other)
{
    struct Enabler : public ParameterString
    {
        Enabler(const ParameterString& other) : ParameterString(other) {}
	};

    return std::make_shared<Enabler>(other);
}

///////////////////////////////////////////////////////////////

void ParameterString::setValue(const std::string& value)
{
    if (isInitialized_ && !isWritable())
    {
        throw std::logic_error("Parameter is not writable");
    }

    std::unique_lock lock(mutex_);
    value_ = value;
};

const std::string& ParameterString::getValue() const
{
    std::shared_lock lock(mutex_);
    return value_;
};

std::shared_ptr<ParameterInfo> ParameterString::getParameterView() const
{
    return std::static_pointer_cast<ParameterInfo>(
        std::make_shared<ParameterStringInfoImpl>(shared_from_this())
    );
}

///////////////////////////////////////////////////////////////////

void ParameterString::setValueImpl(
    const void* value,
    const std::type_info& type)
{
    if (type != typeid(std::string))
        throw std::bad_cast();

    setValue(
        *static_cast<const std::string*>(value)
    );
}


void ParameterString::getValueImpl(
    void* outValue,
    const std::type_info& type) const
{
    if (type != typeid(std::string))
        throw std::bad_cast();

    *static_cast<std::string*>(outValue) = getValue();
}


////////////////////////////////////////////////////////////////


ParameterString::ParameterStringInfoImpl::ParameterStringInfoImpl(const std::shared_ptr<const Parameter>& param)
    : ParameterInfoImpl(param),
    stringParam_(std::static_pointer_cast<const ParameterString>(param))
{
}

////////////////////////////////////////////////////////////////

const std::string& ParameterString::ParameterStringInfoImpl::getValue() const
{
    return stringParam_->getValue();
}