#pragma once
#include "UtilityFunctions.hpp"


template <typename T>
inline void UtilityFunctions::moveInto(
    std::vector<T>&& source,
    std::vector<T>& dest,
    bool moveToBegin)
{
    dest.insert(
        moveToBegin ? dest.begin() : dest.end(),
        std::make_move_iterator(source.begin()),
        std::make_move_iterator(source.end())
    );

    source.clear();
}

template <typename T>
inline void UtilityFunctions::moveInto(
    std::vector<T>& source,
    std::vector<T>& dest,
    bool moveToBegin)
{
    moveInto(std::move(source), dest, moveToBegin);
}

template <typename T>
inline void UtilityFunctions::copyInto(
    const std::vector<T>& source,
    std::vector<T>& dest,
    bool copyToBegin)
{
    dest.insert(
        copyToBegin ? dest.begin() : dest.end(),
        source.begin(),
        source.end()
    );
}

template <typename Enum>
inline std::optional<Enum> UtilityFunctions::enumFromInt(int v)
{
    for (auto& [e, _] : EnumTraits<Enum>::values)
    {
        if (static_cast<int>(e) == v)
            return e;
    }

    return std::nullopt;
}

template <typename Enum>
inline std::optional<Enum> UtilityFunctions::enumFromString(const std::string& s)
{
    for (auto& [e, name] : EnumTraits<Enum>::values)
    {
        if (s == name)
            return e;
    }

    return std::nullopt;
}

template <typename Enum>
inline std::optional<std::string> UtilityFunctions::stringFromEnum(Enum e)
{
    for (auto& [v, name] : EnumTraits<Enum>::values)
    {
        if (v == e)
            return name;
    }

    return std::nullopt;
}

template <typename Enum>
inline std::unordered_map<Enum, std::string> UtilityFunctions::enumValuesToEnumMap()
{
    std::unordered_map<Enum, std::string> map;

    for (auto& [e, s] : EnumTraits<Enum>::values)
    {
        map.emplace(e, s);
    }

    return map;
}

template <typename Enum>
inline std::unordered_map<int, std::string> UtilityFunctions::enumValuesToIntMap()
{
    std::unordered_map<int, std::string> map;

    for (auto& [e, s] : EnumTraits<Enum>::values)
    {
        map.emplace(static_cast<int>(e), s);
    }

    return map;
}