#pragma once
#include "Parameter.hpp"


template<typename T>
void Parameter::setValue(const T& value)
{
    setValueImpl(&value, typeid(T));
}

template<typename T>
T Parameter::getValue() const
{
    T value;

    getValueImpl(&value, typeid(T));

    return value;
}