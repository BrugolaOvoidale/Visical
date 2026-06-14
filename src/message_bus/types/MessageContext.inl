#pragma once
#include "MessageContext.hpp"


template<typename T>
std::enable_if_t<std::is_integral_v<T> && !std::is_same_v<T, bool>, MessageContext&>
MessageContext::setAttribute(const std::string& key, T value)
{
    attributes_[key] = static_cast<int64_t>(value);

    return *this;
}

template<typename T>
std::enable_if_t<std::is_floating_point_v<T>, MessageContext&>
MessageContext::setAttribute(const std::string& key, T value)
{
    attributes_[key] = static_cast<double>(value);

    return *this;
}

template<typename T>
T MessageContext::getAttribute(const std::string& key) const
{
    std::optional<T> opt = tryGetAttribute<T>(key);

    if (!opt.has_value())
    {
        throw std::bad_variant_access();
    }

    return opt.value();
}

template<typename T>
std::optional<T> MessageContext::tryGetAttribute(const std::string& key) const
{
    auto it = attributes_.find(key);
    if (it == attributes_.end()) return std::nullopt;

    // Handle integer requests
    if constexpr (std::is_integral_v<T> && !std::is_same_v<T, bool>)
    {
        if (auto* val = std::get_if<int64_t>(&it->second))
        {
            return static_cast<T>(*val);
        }
    }
    // Handle floating point requests
    else if constexpr (std::is_floating_point_v<T>)
    {
        if (auto* val = std::get_if<double>(&it->second))
        {
            return static_cast<T>(*val);
        }
    }
    // Direct match for bool and string
    else
    {
        if (auto* val = std::get_if<T>(&it->second))
        {
            return *val;
        }
    }

    return std::nullopt;
}