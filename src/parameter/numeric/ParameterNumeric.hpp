#pragma once
#include "../Parameter.hpp"
#include "ParameterNumericInfo.hpp"


/**
 * @brief   Arithmetic-specific implementation of the Parameter class.
 *          It supports only 'int' and 'double' types.
 */
template<typename T>
class ParameterNumeric : public Parameter
{
    static_assert(
        std::is_same_v<T, int> ||
        std::is_same_v<T, double>,
        "ParameterNumeric only supports int and double"
        );

public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    virtual ~ParameterNumeric() = default;


    //-----------------------------------------------------------------------------
    // Construction from factories
    //-----------------------------------------------------------------------------

    static std::shared_ptr<ParameterNumeric> create(
        const std::string& name,
        const std::string& category,
        AccessMode accessMode,
        VisibilityLevel visibilityLevel,
        T value,
        std::optional<T> min = std::nullopt,
        std::optional<T> max = std::nullopt,
        std::optional<T> increment = std::nullopt,
        const std::optional<std::string>& displayName = std::nullopt,
        const std::optional<std::string>& unit = std::nullopt,
        const std::optional<std::string>& description = std::nullopt
    );

    static std::shared_ptr<ParameterNumeric> create(const ParameterNumeric& other);


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    void setValue(T newValue);

    T getValue() const;

    std::shared_ptr<ParameterInfo> getParameterView() const override;

    std::optional<T> getMin() const;

    std::optional<T> getMax() const;

    std::optional<T> getIncrement() const;

protected:
    ParameterNumeric(
        const std::string& name,
        const std::string& category,
        AccessMode accessMode,
        VisibilityLevel visibilityLevel,
        T value,
        std::optional<T> min = std::nullopt,
        std::optional<T> max = std::nullopt,
        std::optional<T> increment = std::nullopt,
        const std::optional<std::string>& displayName = std::nullopt,
        const std::optional<std::string>& unit = std::nullopt,
        const std::optional<std::string>& description = std::nullopt
    );

    ParameterNumeric(const ParameterNumeric& other);

private:
    void setValueImpl(
        const void* value,
        const std::type_info& type
    ) override;

    void getValueImpl(
        void* outValue,
        const std::type_info& type
    ) const override;

private:
    T value_;

    const std::optional<T> minValue_;

    const std::optional<T> maxValue_;

    const std::optional<T> increment_;

private:
#pragma warning(push)
#pragma warning(disable : 4250)
    class ParameterNumericInfoImpl : public ParameterNumericInfo<T>, public ParameterInfoImpl
    {
    public:
        ParameterNumericInfoImpl(const std::shared_ptr<const Parameter>& param);

    private:
        T getValue() const override;

        std::optional<T> getMin() const override;

        std::optional<T> getMax() const override;

        std::optional<T> getIncrement() const override;

    private:
        const std::shared_ptr<const ParameterNumeric<T>> numericParam_;
    };
#pragma warning(pop)
};


#include "ParameterNumeric.inl"


// Int
using ParameterInt = ParameterNumeric<int>;

// Double
using ParameterDouble = ParameterNumeric<double>;