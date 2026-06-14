#pragma once
#include <optional>
#include <unordered_map>
#include "../Parameter.hpp"
#include "ParameterEnumInfo.hpp"


/**
 * @brief Enumeration-specific (bidirectional map of 'int <-> string') implementation of the Parameter class.
 */
class ParameterEnum : public Parameter
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    ~ParameterEnum() = default;


    //-----------------------------------------------------------------------------
    // Construction from factories
    //-----------------------------------------------------------------------------
    
    static std::shared_ptr<ParameterEnum> create(
        const std::string& name,
        const std::string& category,
        AccessMode accessMode,
        VisibilityLevel visibilityLevel,
        std::unordered_map<int, std::string> possibleValues,
        int value,
        const std::optional<std::string>& displayName = std::nullopt,
        const std::optional<std::string>& unit = std::nullopt,
        const std::optional<std::string>& description = std::nullopt
    );

    static std::shared_ptr<ParameterEnum> create(
        const std::string& name,
        const std::string& category,
        AccessMode accessMode,
        VisibilityLevel visibilityLevel,
        std::unordered_map<int, std::string> possibleValues,
        const std::string& value,
        const std::optional<std::string>& displayName = std::nullopt,
        const std::optional<std::string>& unit = std::nullopt,
        const std::optional<std::string>& description = std::nullopt
    );

    static std::shared_ptr<ParameterEnum> create(const ParameterEnum& other);


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    void setValue(int newValue);

    void setValue(const std::string& newValue);

    int getValue() const;

    const std::string& getStrValue() const;

    std::shared_ptr<ParameterInfo> getParameterView() const override;

    std::optional<int> getStrValueAsInt(const std::string& value) const;

    std::optional<std::string> getIntValueAsStr(int value) const;

    const std::unordered_map<int, std::string>& getPossibleIntValues() const;

    const std::unordered_map<std::string, int>& getPossibleStrValues() const;

private:
    ParameterEnum(
        const std::string& name,
        const std::string& category,
        AccessMode accessMode,
        VisibilityLevel visibilityLevel,
        std::unordered_map<int, std::string> possibleValues,
        int value,
        const std::optional<std::string>& displayName = std::nullopt,
        const std::optional<std::string>& unit = std::nullopt,
        const std::optional<std::string>& description = std::nullopt
    );

    ParameterEnum(
        const std::string& name,
        const std::string& category,
        AccessMode accessMode,
        VisibilityLevel visibilityLevel,
        std::unordered_map<int, std::string> possibleValues,
        const std::string& value,
        const std::optional<std::string>& displayName = std::nullopt,
        const std::optional<std::string>& unit = std::nullopt,
        const std::optional<std::string>& description = std::nullopt
    );

    ParameterEnum(const ParameterEnum& other);

    void setValueImpl(
        const void* value,
        const std::type_info& type
    ) override;

    void getValueImpl(
        void* outValue,
        const std::type_info& type
    ) const override;

private:
    int value_;

    std::unordered_map<int, std::string> possibleIntValues_;

    std::unordered_map<std::string, int> possibleStrValues_;

private:
#pragma warning(push)
#pragma warning(disable : 4250)
    // Concrete implementation of ParameterInfo used to provide a read-only interface to a enum Parameter.
    class ParameterEnumInfoImpl : public ParameterEnumInfo, public ParameterInfoImpl
    {
    public:
        ParameterEnumInfoImpl(const std::shared_ptr<const Parameter>& param);

    private:
        int getValue() const override;

        const std::string& getStrValue() const override;

        const std::unordered_map<int, std::string>& getPossibleIntValues() const override;

        const std::unordered_map<std::string, int>& getPossibleStrValues() const override;

    private:
        const std::shared_ptr<const ParameterEnum> enumParam_;
    };
#pragma warning(pop)
};