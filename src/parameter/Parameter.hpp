#pragma once
#include <memory>
#include <shared_mutex>
#include <mutex>
#include "ParameterInfo.hpp"


/**
 * @brief Abstract base class for a thread-safe unified parameter system.
 * 
 * This class provides a generic interface for parameters with metadata.
 * It uses type-erased internal methods to allow derived classes to store
 * various data types while providing public API.
 */
class Parameter : public std::enable_shared_from_this<Parameter>
{
public:
    //-----------------------------------------------------------------------------
    // Constants
    //-----------------------------------------------------------------------------

    /** @brief Separator used to represent hierarchical categories (e.g., "XXX.YYY"). */
    static inline const std::string CATEGORY_SEP{ "." };


    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    virtual ~Parameter() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /**
     * @brief Sets the parameter value.
     * @tparam T The type of the value being passed.
     * @param value The new value to assign.
     * @throw   std::bad_cast If the type T does not match the parameter's internal type.
	 * @throw   std::logic_error if parameter is not writable due to access mode restrictions.
     */
    template<typename T>
    void setValue(const T& value);

    /**
     * @brief Retrieves the parameter value.
     * @tparam T The expected return type.
     * @return The current value cast to type T.
     * @throw std::bad_cast If the type T does not match the parameter's internal type.
     */
    template<typename T>
    T getValue() const;

    /**
     * @brief Generates a read-only view of the parameter.
     * @return A shared pointer to a ParameterInfo object.
     */
    virtual std::shared_ptr<ParameterInfo> getParameterView() const;

    /** @brief Returns the unique identifier name of the parameter. */
    const std::string& name() const;

    /** @brief Returns the hierarchical category this parameter belongs to. */
    const std::string& category() const;
    
    /** @brief Returns the intrinsic data type */
    DataType dataType() const;
    
    /** @brief Returns the inherent access mode */
    AccessMode accessMode() const;

    /**
     * @brief Overrides the default access mode.
     * @param mode The new access mode to impose.
     * @param reason Optional string explaining why the mode is imposed.
     */
    void setImposedAccessMode(
        AccessMode mode,
        const std::optional<std::string>& reason
    );

    /** @brief Removes the imposed access mode and reverts to the original state. */
    void resetImposedAccessMode();

    /**
     * @brief Returns the current effective access mode.
     * This may be more restrictive than 'getAccessMode()' due to system state.
     */
    AccessMode imposedAccessMode() const;

    /** @brief Returns a human-readable reason for the current access mode override. */
    std::optional<std::string> imposedAccessModeReason() const;
    
    /** @brief Returns the logical visibility level. */
    VisibilityLevel visibilityLevel() const;

    /** @brief Returns a user-friendly name, if available. */
    const std::optional<std::string>& displayName() const;

    /** @brief Returns the physical unit, if applicable. */
    const std::optional<std::string>& unit() const;

    /** @brief Returns a detailed description of the parameter, if available. */
    const std::optional<std::string>& description() const;

    /** @brief Convenience method to check if the parameter can currently be written to. */
    bool isWritable() const;

protected:
    // Protected constructor for derived classes to specify the internal DataType.
    Parameter(
        const std::string& name,
        const std::string& category,
        DataType dataType,
        AccessMode accessMode,
        VisibilityLevel visibilityLevel,
        const std::optional<std::string>& displayName = std::nullopt,
        const std::optional<std::string>& unit = std::nullopt,
        const std::optional<std::string>& description = std::nullopt
    );

    Parameter(const Parameter& other);


    /**
     * @brief Internal implementation for setting values.
     * @param value Pointer to the raw data.
     * @param type Type information of the passed data for validation.
     * @throw   std::bad_cast If the type T does not match the parameter's internal type.
	 * @throw   std::logic_error if parameter is not writable due to access mode restrictions.
     */
    virtual void setValueImpl(
        const void* value,
        const std::type_info&
    ) = 0;

    /**
     * @brief Internal implementation for getting values.
     * @param value Pointer to the destination memory.
     * @param type Type information of the requested type for validation.
     */
    virtual void getValueImpl(
        void* value,
        const std::type_info&
    ) const = 0;

protected:
    // Mutex protecting access to the internal value in derived classes.
    mutable std::shared_mutex mutex_;

    // Flag indicating if the parameter has been assigned an initial value.
    bool isInitialized_ = false;

private:
    std::string name_;

    std::string category_;

    DataType dataType_;

    AccessMode accessMode_;

    AccessMode imposedAccessMode_;

    std::optional<std::string> imposedAccessModeReason_;

    VisibilityLevel visibilityLevel_;

    std::optional<std::string> displayName_;

    std::optional<std::string> unit_;

    std::optional<std::string> description_;

protected:
    // Concrete implementation of ParameterInfo used to provide a read-only interface to a Parameter.
    class ParameterInfoImpl : public virtual ParameterInfo
    {
    public:
        /** @throw std::invalid_argument if param is nullptr. */
        ParameterInfoImpl(const std::shared_ptr<const Parameter>& param);

    private:
        void getValueImpl(
            void* value,
            const std::type_info& type
        ) const override;

        const std::string& name() const override;

        const std::string& category() const override;

        DataType dataType() const override;

        AccessMode accessMode() const override;

		AccessMode imposedAccessMode() const override;

        std::optional<std::string> imposedAccessModeReason() const override;

        VisibilityLevel visibilityLevel() const override;

        const std::optional<std::string>& displayName() const override;

        const std::optional<std::string>& unit() const override;

        const std::optional<std::string>& description() const override;

        bool isWritable() const override;

    private:
        const std::shared_ptr<const Parameter> param_;
    };
};

///////////////////////////////////////////////////////////

// Key used for identifying parameters in hash maps, combining Id and category. */
struct ParamKey {
    std::string paramId;
    std::string categoryId;

    bool operator==(const ParamKey& other) const noexcept {
        return paramId == other.paramId && categoryId == other.categoryId;
    }
};

// Hash provider for ParamKey.
struct ParamKeyHash {
    std::size_t operator()(const ParamKey& k) const noexcept {
        std::size_t h1 = std::hash<std::string>{}(k.paramId);
        std::size_t h2 = std::hash<std::string>{}(k.categoryId);
        return h1 ^ (h2 << 1);
    }
};


#include "Parameter.inl"