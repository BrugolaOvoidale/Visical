#pragma once
#include "../ParameterInfo.hpp"


/**
 * @brief Arithmetic-specific implementation of the ParameterInfo class.
 */
template<typename T>
class ParameterNumericInfo : public virtual ParameterInfo
{
    static_assert(
        std::is_same_v<T, int> ||
        std::is_same_v<T, double>,
        "ParameterNumeric only supports int and double"
        );

public:
    ParameterNumericInfo() = default;

    ~ParameterNumericInfo() = default;

    virtual T getValue() const = 0;

    virtual std::optional<T> getMin() const = 0;

    virtual std::optional<T> getMax() const = 0;

    virtual std::optional<T> getIncrement() const = 0;
};

// Int
using ParameterIntInfo = ParameterNumericInfo<int>;

// Double
using ParameterDoubleInfo = ParameterNumericInfo<double>;
