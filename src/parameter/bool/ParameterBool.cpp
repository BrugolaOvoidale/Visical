#include "ParameterBool.hpp"


ParameterBool::ParameterBool(
    const std::string& name,
    const std::string& category,
    AccessMode accessMode,
    VisibilityLevel visibilityLevel,
    bool value,
    const std::optional<std::string>& displayName,
    const std::optional<std::string>& unit,
    const std::optional<std::string>& description
) : Parameter(name, category, DataType::BOOLEAN, accessMode, visibilityLevel, displayName, unit, description)
{
    setValue(value);

    isInitialized_ = true;
}

ParameterBool::ParameterBool(const ParameterBool& other)
: Parameter(other), value_(other.getValue())
{
    isInitialized_ = true;
}

///////////////////////////////////////////////////////////////

std::shared_ptr<ParameterBool> ParameterBool::create(
    const std::string& name,
    const std::string& category,
    AccessMode accessMode,
    VisibilityLevel visibilityLevel,
    bool value,
    const std::optional<std::string>& displayName,
    const std::optional<std::string>& unit,
    const std::optional<std::string>& description)
{
    struct Enabler : public ParameterBool
    {
        Enabler(
            const std::string& name,
            const std::string& category,
            AccessMode accessMode,
            VisibilityLevel visibilityLevel,
            bool value,
            const std::optional<std::string>& displayName,
            const std::optional<std::string>& unit,
            const std::optional<std::string>& description)
        : ParameterBool(name, category, accessMode, visibilityLevel, value, displayName, unit, description) {}
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

std::shared_ptr<ParameterBool> ParameterBool::create(const ParameterBool& other)
{
    struct Enabler : public ParameterBool
    {
        Enabler(const ParameterBool& other) : ParameterBool(other) {}
	};

    return std::make_shared<Enabler>(other);
}

///////////////////////////////////////////////////////////////

std::shared_ptr<ParameterInfo> ParameterBool::getParameterView() const
{
    return std::static_pointer_cast<ParameterInfo>(
        std::make_shared<ParameterBoolInfoImpl>(shared_from_this())
    );
}

void ParameterBool::setValue(bool value)
{
    if (isInitialized_ && !isWritable())
    {
        throw std::logic_error("Parameter is not writable");
    }

    std::unique_lock lock(mutex_);
    value_ = value;
}

bool ParameterBool::getValue() const
{
    std::shared_lock lock(mutex_);
    return value_;
};


///////////////////////////////////////////////////////////////////

void ParameterBool::setValueImpl(
    const void* value,
    const std::type_info& type)
{
    if (type != typeid(bool))
        throw std::bad_cast();

    setValue(
        *static_cast<const bool*>(value)
    );
}


void ParameterBool::getValueImpl(
    void* outValue,
    const std::type_info& type) const
{
    if (type != typeid(bool))
        throw std::bad_cast();

    *static_cast<bool*>(outValue) = getValue();
}


///////////////////////////////////////////////////////////////////


ParameterBool::ParameterBoolInfoImpl::ParameterBoolInfoImpl(const std::shared_ptr<const Parameter>& param)
    : ParameterInfoImpl(param), boolParam_(std::static_pointer_cast<const ParameterBool>(param))
{
}

bool ParameterBool::ParameterBoolInfoImpl::getValue() const
{
    return boolParam_->getValue();
}