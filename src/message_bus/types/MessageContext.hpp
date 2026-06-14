#pragma once
#include <variant>
#include <string>
#include <cstdint>
#include <unordered_map>
#include <optional>


/**
 * @brief A container for heterogeneous attributes used to provide context to messages.
 *
 * This class stores metadata as key-value pairs where keys are strings and values
 * are a variant of standard types (int64_t, double, bool, or string).
 */
class MessageContext
{
public:
    //-----------------------------------------------------------------------------
    // Aliases
    //-----------------------------------------------------------------------------

    /**
     * @brief Type alias for the supported value types.
     * - std::int64_t: For all integral types (except bool).
     * - double: For all floating-point types.
     * - bool: For boolean flags.
     * - std::string: For text data.
     */
    using Attribute = std::variant<
        std::int64_t,     // ALL integers (int, long, size_t, etc.)
        double,      // ALL floating point (float, double)
        bool,
        std::string
    >;


    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    /**
     * @brief Constructs a context with an optional map of attributes.
     * @param attributes Initial map of string keys and variant values.
     */
    MessageContext(const std::unordered_map<std::string, Attribute>& attributes = {});

    /**
     * @brief Convinience constructor using an initializer list.
     * 
     * E.g.: MessageContext ctx = {{"id", 10LL}, {"name", "sensor_1"}};
     */
    MessageContext(std::initializer_list<std::pair<const std::string, Attribute>> init);

    MessageContext(const MessageContext&) = default;

    ~MessageContext() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /**
     * @brief Replaces the entire attribute set.
     * @param attributes The new map of attributes.
     * @return Reference to this for chaining.
     */
    MessageContext& setAttributes(const std::unordered_map<std::string, Attribute>& attributes);

    /**
     * @brief Sets an integral attribute.
     * @tparam T Any integral type (int, long, size_t) except bool.
     * @note Value is internally cast to std::int64_t.
     */
    template<typename T>
    std::enable_if_t<std::is_integral_v<T> && !std::is_same_v<T, bool>, MessageContext&>
        setAttribute(const std::string& key, T value);

    /**
     * @brief Sets a floating point attribute.
     * @tparam T float or double.
     */
    template<typename T>
    std::enable_if_t<std::is_floating_point_v<T>, MessageContext&>
        setAttribute(const std::string& key, T value);

    /** @brief Sets a boolean attribute. */
    MessageContext& setAttribute(const std::string& key, bool value);

    /** @brief Sets a string attribute. */
    MessageContext& setAttribute(const std::string& key, const std::string& value);

    /**
     * @brief Attempts to retrieve an attribute of a specific type.
     * @tparam T The expected type.
     * @return An optional containing the value if the key exists and the type matches.
     */
    template<typename T>
    std::optional<T> tryGetAttribute(const std::string& key) const;

    /**
     * @brief Retrieves an attribute or throws if not found/type mismatch.
     * @tparam T The expected type.
     * @return The value of type T.
     * @throw std::bad_variant_access
     */
    template<typename T>
    T getAttribute(const std::string& key) const;

    /** @brief Retrieves the raw variant value if the key exists. */
    std::optional<Attribute> getVariantAttribute(const std::string& key) const;

    /** @brief Returns the full map of attributes. */
    const std::unordered_map<std::string, Attribute>& getAttributes() const;

private:
    // Attributes answer: WHAT are the details?
    // Future: priority, etc. ?
    std::unordered_map<std::string, Attribute> attributes_;
};

#include "MessageContext.inl"
