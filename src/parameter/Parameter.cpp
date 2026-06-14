#include "Parameter.hpp"
#include <stdexcept>


Parameter::Parameter(
    const std::string& name,
    const std::string& category,
	DataType dataType,
    AccessMode accessMode,
    VisibilityLevel visibilityLevel,
    const std::optional<std::string>& displayName,
    const std::optional<std::string>& unit,
    const std::optional<std::string>& description)
    : name_(name),
    category_(category),
    dataType_(dataType),
    accessMode_(accessMode),
    imposedAccessMode_(accessMode),
    visibilityLevel_(visibilityLevel),
    displayName_(displayName),
    unit_(unit),
    description_(description)
{
}

Parameter::Parameter(const Parameter& other)
    : name_(other.name_),
    category_(other.category_),
    dataType_(other.dataType_),
    accessMode_(other.accessMode_),
    imposedAccessMode_(other.accessMode_),
    visibilityLevel_(other.visibilityLevel_),
    displayName_(other.displayName_),
    unit_(other.unit_),
	description_(other.description_)
{
}

////////////////////////////////////////////////////////////////

std::shared_ptr<ParameterInfo> Parameter::getParameterView() const
{
    return std::static_pointer_cast<ParameterInfo>(std::make_shared<ParameterInfoImpl>(shared_from_this()));
}

const std::string& Parameter::name() const
{
    return name_;
}

const std::string& Parameter::category() const
{
    return category_;
}

DataType Parameter::dataType() const
{
    return dataType_;
}

AccessMode Parameter::accessMode() const
{
    return accessMode_;
}

void Parameter::setImposedAccessMode(
    AccessMode mode,
    const std::optional<std::string>& reason)
{
    std::unique_lock lock(mutex_);

    imposedAccessMode_ = mode;

    if (reason.has_value())
        imposedAccessModeReason_ = reason.value();
}

void Parameter::resetImposedAccessMode()
{
    std::unique_lock lock(mutex_);

    imposedAccessMode_ = accessMode_;

    imposedAccessModeReason_.reset();
}

AccessMode Parameter::imposedAccessMode() const
{
    std::shared_lock lock(mutex_);

    return imposedAccessMode_;
}

std::optional<std::string> Parameter::imposedAccessModeReason() const
{
    std::shared_lock lock(mutex_);

    return imposedAccessModeReason_;
}

VisibilityLevel Parameter::visibilityLevel() const
{
    return visibilityLevel_;
}

const std::optional<std::string>& Parameter::displayName() const
{
    return displayName_;
}

const std::optional<std::string>& Parameter::unit() const
{
    return unit_;
}

const std::optional<std::string>& Parameter::description() const
{
    return description_;
}

bool Parameter::isWritable() const
{
    return imposedAccessMode_ == AccessMode::READWRITE || imposedAccessMode_ == AccessMode::WRITEONLY;
}


////////////////////////////////////////////////////////////////


Parameter::ParameterInfoImpl::ParameterInfoImpl(const std::shared_ptr<const Parameter>& param) : param_(param)
{
    if (!param)
    {
        throw (std::invalid_argument("param is nullptr"));
    }
}

////////////////////////////////////////////////////////////////

void Parameter::ParameterInfoImpl::getValueImpl(
    void* value,
    const std::type_info& type) const
{
    param_->getValueImpl(value, type);
}

const std::string& Parameter::ParameterInfoImpl::name() const
{
    return param_->name();
}

const std::string& Parameter::ParameterInfoImpl::category() const
{
    return param_->category();
}

DataType Parameter::ParameterInfoImpl::dataType() const
{
    return param_->dataType();
}

AccessMode Parameter::ParameterInfoImpl::accessMode() const
{
    return param_->accessMode();
}

AccessMode Parameter::ParameterInfoImpl::imposedAccessMode() const
{
    return param_->imposedAccessMode();
}

std::optional<std::string> Parameter::ParameterInfoImpl::imposedAccessModeReason() const
{
    return param_->imposedAccessModeReason();
}

VisibilityLevel Parameter::ParameterInfoImpl::visibilityLevel() const
{
    return param_->visibilityLevel();
}

const std::optional<std::string>& Parameter::ParameterInfoImpl::displayName() const
{
    return param_->displayName();
}

const std::optional<std::string>& Parameter::ParameterInfoImpl::unit() const
{
    return param_->unit();
}

const std::optional<std::string>& Parameter::ParameterInfoImpl::description() const
{
    return param_->description();
}

bool Parameter::ParameterInfoImpl::isWritable() const
{
    return param_->isWritable();
}