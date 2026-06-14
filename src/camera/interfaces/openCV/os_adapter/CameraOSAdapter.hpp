#pragma once
#include <string>
#include <vector>


/**
 * @brief Base template for platform-specific camera enumeration.
 * 
 * Uses the Curiously Recurring Template Pattern (CRTP) to provide a static
 * interface for operating system adapters. Currently DirectShow (Windows) and V4L2 (Linux).
 * * @tparam Derived The specific OS implementation class.
 */
template<typename Derived>
class CameraOSAdapter
{
public:
    /**
     * @brief Retrieves a list of unique hardware identifiers for available cameras.
     * @return A vector of strings containing device paths or hardware IDs.
     */
    static std::vector<std::string> listDeviceIds();
};

#include "CameraOSAdapter.inl"