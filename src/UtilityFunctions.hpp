#pragma once
#include <vector>
#include <string>
#include <optional>
#include <unordered_map>
#include <enum/EnumTraits.hpp>


/**
 * @brief A collection of static helper methods.
 */
class UtilityFunctions
{
public:
    /**
     * @brief Moves all elements from a source vector into a destination vector.
     * @tparam T The element type.
     * @param[in,out] source The vector to move from (will be empty after call).
     * @param[in,out] dest The vector to move into.
     * @param moveToBegin If true, elements are inserted at the start; otherwise, at the end.
     */
    template <typename T>
    static void moveInto(
        std::vector<T>&& source,
        std::vector<T>& dest,
        bool moveToBegin = false
    );
    template <typename T>
    static void moveInto(
        std::vector<T>& source,
        std::vector<T>& dest,
        bool moveToBegin = false
    );

    /**
     * @brief Copies all elements from a source vector into a destination vector.
     * @tparam T The element type.
     * @param source The vector to copy from.
     * @param[in,out] dest The vector to copy into.
     * @param copyToBegin If true, elements are inserted at the start; otherwise, at the end.
     */
    template <typename T>
    static void copyInto(
        const std::vector<T>& source,
        std::vector<T>& dest,
        bool copyToBegin = false
    );

    /**
     * @brief Converts an integer to an Enum value.
     * @tparam Enum The enum type (must be supported by EnumTraits).
     * @param v The integer value to convert.
     * @param fallback The value to return if the integer is not a valid enum member.
     * @return The corresponding enum value or fallback.
     */
    template <typename Enum>
    static std::optional<Enum> enumFromInt(int v);

    /**
     * @brief Converts a string to an Enum value.
     * @tparam Enum The enum type (must be supported by EnumTraits).
     * @param s The string representation of the enum.
     * @param fallback The value to return if the string is not recognized.
     * @return The corresponding enum value or fallback.
     */
    template <typename Enum>
    static std::optional<Enum> enumFromString(const std::string& s);

    /**
     * @brief Retrieves the string name of an enum value.
     * @tparam Enum The enum type.
     * @param e The enum value.
     * @return An optional containing the string name, or std::nullopt if not found.
     */
    template <typename Enum>
    static std::optional<std::string> stringFromEnum(Enum e);

    /**
     * @brief Generates a map of all enum values to their string names.
     * @tparam Enum The enum type.
     * @return A map where keys are enum constants and values are their string names.
     */
    template <typename Enum>
    static std::unordered_map<Enum, std::string> enumValuesToEnumMap();

    /**
     * @brief Generates a map of enum integer values to their string names.
     * @tparam Enum The enum type.
     * @return A map where keys are underlying integer values and values are their string names.
     */
    template <typename Enum>
    static std::unordered_map<int, std::string> enumValuesToIntMap();
};

#include "UtilityFunctions.inl"