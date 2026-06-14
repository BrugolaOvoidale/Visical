#pragma once
#include "ParameterNumeric.hpp"


template<typename T>
ParameterNumeric<T>::ParameterNumeric(
    const std::string& name,
    const std::string& category,
    AccessMode accessMode,
    VisibilityLevel visibilityLevel,
    T value,
    std::optional<T> min,
    std::optional<T> max,
    std::optional<T> increment,
    const std::optional<std::string>& displayName,
    const std::optional<std::string>& unit,
    const std::optional<std::string>& description)
    : Parameter(name, category, DataTypeTraits<T>::value, accessMode, visibilityLevel, displayName, unit, description),
    minValue_(min), maxValue_(max), increment_(increment)
{
    // Validate value against domain constraints
    setValue(value);

    isInitialized_ = true;
}

template<typename T>
ParameterNumeric<T>::ParameterNumeric(const ParameterNumeric& other)
    : Parameter(other),
    minValue_(other.getMin()), maxValue_(other.getMax()), increment_(other.getIncrement()), value_(other.getValue())
{
    isInitialized_ = true;
}

////////////////////////////////////////////////////////////////////

template<typename T>
std::shared_ptr<ParameterNumeric<T>> ParameterNumeric<T>::create(
    const std::string& name,
    const std::string& category,
    AccessMode accessMode,
    VisibilityLevel visibilityLevel,
    T value,
    std::optional<T> min,
    std::optional<T> max,
    std::optional<T> increment,
    const std::optional<std::string>& displayName,
    const std::optional<std::string>& unit,
    const std::optional<std::string>& description)
{
    struct Enabler : ParameterNumeric<T>
    {
        Enabler(const std::string& name,
            const std::string& category,
            AccessMode accessMode,
            VisibilityLevel visibilityLevel,
            T value,
            std::optional<T> min,
            std::optional<T> max,
            std::optional<T> increment,
            const std::optional<std::string>& displayName,
            const std::optional<std::string>& unit,
            const std::optional<std::string>& description)
            : ParameterNumeric<T>(name,
                category,
                accessMode,
                visibilityLevel,
                value,
                min,
                max,
                increment,
                displayName,
                unit,
                description)
        {
        }
    };

    return std::make_shared<Enabler>(
        name, category, accessMode, visibilityLevel,
        value, min, max, increment,
        displayName, unit, description
    );
}

template<typename T>
std::shared_ptr<ParameterNumeric<T>> ParameterNumeric<T>::create(const ParameterNumeric& other)
{
    struct Enabler : ParameterNumeric<T>
    {
        Enabler(const ParameterNumeric<T>& other)
            : ParameterNumeric<T>(other)
        {
        }
    };

    return std::make_shared<Enabler>(other);
}

////////////////////////////////////////////////////////////////////

template<typename T>
void ParameterNumeric<T>::setValue(T newValue)
{
    if (isInitialized_ && !isWritable())
    {
        throw std::logic_error("Parameter is not writable");
    }

    std::unique_lock lock(mutex_);
    value_ = newValue;
}

template<typename T>
T ParameterNumeric<T>::getValue() const
{
    std::shared_lock lock(mutex_);
    return value_;
};

template<typename T>
std::shared_ptr<ParameterInfo> ParameterNumeric<T>::getParameterView() const
{
    return std::static_pointer_cast<ParameterInfo>(
        std::make_shared<ParameterNumericInfoImpl>(shared_from_this())
    );
}

template<typename T>
std::optional<T> ParameterNumeric<T>::getMin() const
{
    return minValue_;
};

template<typename T>
std::optional<T> ParameterNumeric<T>::getMax() const
{
    return maxValue_;
};

template<typename T>
std::optional<T> ParameterNumeric<T>::getIncrement() const
{
    return increment_;
};

////////////////////////////////////////////////////////////////////

template<typename T>
void ParameterNumeric<T>::setValueImpl(
    const void* value,
    const std::type_info& type)
{
    if (type != typeid(T))
        throw std::bad_cast();

    setValue(
        *static_cast<const T*>(value)
    );
}

template<typename T>
void ParameterNumeric<T>::getValueImpl(
    void* outValue,
    const std::type_info& type) const
{
    if (type != typeid(T))
        throw std::bad_cast();

    *static_cast<T*>(outValue) = getValue();
}


////////////////////////////////////////////////////////////////////


template<typename T>
ParameterNumeric<T>::ParameterNumericInfoImpl::ParameterNumericInfoImpl(
    const std::shared_ptr<const Parameter>& param)
    : ParameterInfoImpl(param),
    numericParam_(std::static_pointer_cast<const ParameterNumeric<T>>(param))
{
}

////////////////////////////////////////////////////////////////////

template<typename T>
T ParameterNumeric<T>::ParameterNumericInfoImpl::getValue() const
{
    return numericParam_->getValue();
};

template<typename T>
std::optional<T> ParameterNumeric<T>::ParameterNumericInfoImpl::getMin() const
{
    return numericParam_->getMin();
}

template<typename T>
std::optional<T> ParameterNumeric<T>::ParameterNumericInfoImpl::getMax() const
{
    return numericParam_->getMax();
}

template<typename T>
std::optional<T> ParameterNumeric<T>::ParameterNumericInfoImpl::getIncrement() const
{
    return numericParam_->getIncrement();
}