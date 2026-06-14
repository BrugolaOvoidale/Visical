#pragma once
#include "../Parameter.hpp"
#include "ParameterBoolInfo.hpp"


/**
 * @brief Bool-specific implementation of the Parameter class.
 */
class ParameterBool : public Parameter
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    ~ParameterBool() = default;


    //-----------------------------------------------------------------------------
    // Construction from factories
    //-----------------------------------------------------------------------------

    static std::shared_ptr<ParameterBool> create(
        const std::string& name,
        const std::string& category,
        AccessMode accessMode,
        VisibilityLevel visibilityLevel,
        bool value,
        const std::optional<std::string>& displayName = std::nullopt,
        const std::optional<std::string>& unit = std::nullopt,
        const std::optional<std::string>& description = std::nullopt
    );

    static std::shared_ptr<ParameterBool> create(const ParameterBool& other);


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    void setValue(bool value);

    bool getValue() const;

    std::shared_ptr<ParameterInfo> getParameterView() const override;

private:
    ParameterBool(
        const std::string& name,
        const std::string& category,
        AccessMode accessMode,
        VisibilityLevel visibilityLevel,
        bool value,
        const std::optional<std::string>& displayName = std::nullopt,
        const std::optional<std::string>& unit = std::nullopt,
        const std::optional<std::string>& description = std::nullopt
    );

    ParameterBool(const ParameterBool& other);

    void setValueImpl(
        const void* value,
        const std::type_info& type
    ) override;

    void getValueImpl(
        void* outValue,
        const std::type_info& type
    ) const override;

private:
    bool value_;

private:
#pragma warning(push)
#pragma warning(disable : 4250)
    class ParameterBoolInfoImpl : public ParameterBoolInfo, public ParameterInfoImpl
    {
    public:
        ParameterBoolInfoImpl(const std::shared_ptr<const Parameter>& param);

    private:
        bool getValue() const override;

    private:
        const std::shared_ptr<const ParameterBool> boolParam_;
    };
#pragma warning(pop)
};
