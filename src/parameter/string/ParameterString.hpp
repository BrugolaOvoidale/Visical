#pragma once
#include "../Parameter.hpp"
#include "ParameterStringInfo.hpp"


/**
 * @brief String-specific implementation of the Parameter class.
 */
class ParameterString : public Parameter
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    ~ParameterString() = default;


    //-----------------------------------------------------------------------------
    // Construction from factories
    //-----------------------------------------------------------------------------

    static std::shared_ptr<ParameterString> create(
        const std::string& name,
        const std::string& category,
        AccessMode accessMode,
        VisibilityLevel visibilityLevel,
        const std::string& value,
        const std::optional<std::string>& displayName = std::nullopt,
        const std::optional<std::string>& unit = std::nullopt,
        const std::optional<std::string>& description = std::nullopt
    );

    static std::shared_ptr<ParameterString> create(const ParameterString& other);


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    void setValue(const std::string& value);

    const std::string& getValue() const;

    std::shared_ptr<ParameterInfo> getParameterView() const override;

private:
    ParameterString(
        const std::string& name,
        const std::string& category,
        AccessMode accessMode,
        VisibilityLevel visibilityLevel,
        const std::string& value,
        const std::optional<std::string>& displayName = std::nullopt,
        const std::optional<std::string>& unit = std::nullopt,
        const std::optional<std::string>& description = std::nullopt
    );

    ParameterString(const ParameterString& other);

    void setValueImpl(
        const void* value,
        const std::type_info& type
    ) override;

    void getValueImpl(
        void* outValue,
        const std::type_info& type
    ) const override;

private:
    std::string value_;

private:
#pragma warning(push)
#pragma warning(disable : 4250)
    // Concrete implementation of ParameterInfo used to provide a read-only interface to a string Parameter.
    class ParameterStringInfoImpl : public ParameterStringInfo, public ParameterInfoImpl
    {
    public:
        ParameterStringInfoImpl(const std::shared_ptr<const Parameter>& param);

    private:
        const std::string& getValue() const override;

    private:
        const std::shared_ptr<const ParameterString> stringParam_;
    };
#pragma warning(pop)
};
