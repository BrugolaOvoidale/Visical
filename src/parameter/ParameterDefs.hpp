#pragma once
#include <enum/EnumTraits.hpp>


/**
 * @brief Categorizes the underlying data type of a parameter.
 */
enum class DataType {
    UNDEFINED = -1,     ///< Type not specified or invalid.
    INTEGER,            ///< 32-bit or 64-bit signed integer.
    DOUBLE,             ///< Double precision floating point.
    ENUMERATION,        ///< A set of named discrete values.
    BOOLEAN,            ///< Logical true/false.
    STRING,             ///< Character sequence.
    COMMAND             ///< Represents a trigger or executable action.
};

/**
 * @brief Compile-time mapping from C++ types to DataType.
 * @tparam T The C++ native type.
 */
template<typename T>
struct DataTypeTraits;

template<>
struct DataTypeTraits<int>
{
    static constexpr DataType value = DataType::INTEGER;
};

template<>
struct DataTypeTraits<double>
{
    static constexpr DataType value = DataType::DOUBLE;
};

///////////////////////////////////////////////////////

/**
 * @brief Defines the permission levels for reading from a parameter.
 */
enum class AccessMode {
    UNDEFINED = -1,     ///< Access level unknown.
    READONLY,           ///< Value can be retrieved but not modified.
    WRITEONLY,          ///< Value can be set but not retrieved.
    READWRITE           ///< Full read and write access.
};

/** @brief String representation mapping for AccessMode. */
template <>
struct EnumTraits<AccessMode> {
    static constexpr std::array<std::pair<AccessMode, const char*>, 4> values{ {
    {AccessMode::UNDEFINED,     "Undefined"},
    {AccessMode::READONLY,      "Read only"},
    {AccessMode::WRITEONLY,     "Write only"},
    {AccessMode::READWRITE,     "Read/Write"}
    } };
};

///////////////////////////////////////////////////////

/**
 * @brief Controls the exposure of parameters in the user interface based on user expertise.
 */
enum class VisibilityLevel {
    UNDEFINED = -1,     ///< Visibility not set.
    HIDDEN,             ///< Internal parameter; never shown in UI.
    BASIC,              ///< Standard parameters for all users.
    INTERMEDIATE,       ///< Detailed parameters for experienced users.
    ADVANCED            ///< Low-level configuration for expert users.
};

/** @brief String representation mapping for VisibilityLevel. */
template <>
struct EnumTraits<VisibilityLevel> {
    static constexpr std::array<std::pair<VisibilityLevel, const char*>, 5> values{ {
    {VisibilityLevel::UNDEFINED,        "Undefined"},
    {VisibilityLevel::HIDDEN,           "Hidden"},
    {VisibilityLevel::BASIC,            "Basic"},
    {VisibilityLevel::INTERMEDIATE,     "Intermediate"},
    {VisibilityLevel::ADVANCED,         "Advanced"}
    } };
};