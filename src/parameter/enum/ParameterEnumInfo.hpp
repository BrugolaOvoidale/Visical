#pragma once
#include "../ParameterInfo.hpp"


/**
 * @brief Enumeration-specific implementation of the ParameterInfo class.
 */
class ParameterEnumInfo : public virtual ParameterInfo
{
public:
    ParameterEnumInfo() = default;

    ~ParameterEnumInfo() = default;

    virtual int getValue() const = 0;

    virtual const std::string& getStrValue() const = 0;

    virtual const std::unordered_map<int, std::string>& getPossibleIntValues() const = 0;

    virtual const std::unordered_map<std::string, int>& getPossibleStrValues() const = 0;
};