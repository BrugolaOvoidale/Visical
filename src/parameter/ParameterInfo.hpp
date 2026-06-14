#pragma once
#include <string>
#include <optional>
#include <typeinfo>
#include "ParameterDefs.hpp"


/**
 * @brief Abstract base class representing a generic read-only interface for a parameter or feature.
 */
class ParameterInfo
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    ParameterInfo() = default;

    virtual ~ParameterInfo() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /**
	 * @brief Retrieves the parameter value. See 'Parameter::getValue()' for details.
     * @tparam T The expected return type.
     * @return The current value cast to type T.
     */
    template<typename T>
    T getValue() const;

    /** @brief Returns the unique identifier name of the parameter. */
    virtual const std::string& name() const = 0;

    /** @brief Returns the hierarchical category this parameter belongs to. */
    virtual const std::string& category() const = 0;

    /** @brief Returns the intrinsic data type */
    virtual DataType dataType() const = 0;

    /** @brief Returns the inherent access mode */
    virtual AccessMode accessMode() const = 0;

    /**
     * @brief Returns the current effective access mode.
     * This may be more restrictive than 'getAccessMode()' due to system state.
     */
    virtual AccessMode imposedAccessMode() const = 0;

    /** @brief Returns a human-readable reason for the current access mode override. */
    virtual std::optional<std::string> imposedAccessModeReason() const = 0;

    /** @brief Returns the logical visibility level. */
    virtual VisibilityLevel visibilityLevel() const = 0;

    /** @brief Returns a user-friendly name, if available. */
    virtual const std::optional<std::string>& displayName() const = 0;

    /** @brief Returns the physical unit, if applicable. */
    virtual const std::optional<std::string>& unit() const = 0;

    /** @brief Returns a detailed description of the parameter, if available. */
    virtual const std::optional<std::string>& description() const = 0;

    /** @brief Convenience method to check if the parameter can currently be written to. */
    virtual bool isWritable() const = 0;

protected:
    /**
     * @brief Internal implementation for getting values.
     * @param value Pointer to the destination memory.
     * @param type Type information of the requested type for validation.
     */
    virtual void getValueImpl(
        void* value,
        const std::type_info&
    ) const = 0;
};

#include "ParameterInfo.inl"