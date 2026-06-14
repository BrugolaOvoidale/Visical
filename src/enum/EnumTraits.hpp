#pragma once
#include <array>


/**
 * @brief Primary template for enum metadata.
 * 
 * Specializations of this struct should provide a static list
 * called 'values' containing pairs of enum constants and their string names.
 * 
 * @tparam Enum The enumeration type to describe.
 */
template <typename Enum>
struct EnumTraits; // intentionally undefined