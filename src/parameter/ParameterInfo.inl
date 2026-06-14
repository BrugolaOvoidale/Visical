#pragma once
#include "ParameterInfo.hpp"


template<typename T>
T ParameterInfo::getValue() const
{
    T value;

    getValueImpl(&value, typeid(T));

    return value;
}