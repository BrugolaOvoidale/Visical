#pragma once
#include "../ParameterInfo.hpp"


/**
 * @brief String-specific implementation of the ParameterInfo class.
 */
class ParameterStringInfo : public virtual ParameterInfo
{
public:
    ParameterStringInfo() = default;

    ~ParameterStringInfo() = default;

    virtual const std::string& getValue() const = 0;
};