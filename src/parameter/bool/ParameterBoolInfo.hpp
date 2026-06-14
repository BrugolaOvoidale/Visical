#pragma once
#include "../ParameterInfo.hpp"


/**
 * @brief Bool-specific implementation of the ParameterInfo class.
 */
class ParameterBoolInfo : public virtual ParameterInfo
{
public:
    ParameterBoolInfo() = default;

    ~ParameterBoolInfo() = default;

    virtual bool getValue() const = 0;
};
