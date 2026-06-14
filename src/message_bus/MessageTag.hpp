#pragma once
#include <string_view>
#include <cstdint>


/**
 * @brief Macro to define and initialize a global MessageTag.
 * @param T The data type associated with the message.
 * @param NAME The variable name and string identifier for the tag.
 */
#define REGISTER_MESSAGE(T, NAME) \
    inline const MessageTag<T> NAME{#NAME};


/**
 * @brief Base class for message tags, providing non-templated Id and Name access.
 */
class IMessageTag
{
public:
    /**
     * @brief Wrapper for the numeric message identifier to ensure type safety.
     */
    class Id
    {
    public:
        explicit Id(std::uint32_t id) : id_(id) {}

        ~Id() = default;

        bool operator==(const Id& other) const { return id_ == other.id_; }

        std::uint32_t get() const { return id_; }

    private:
        std::uint32_t id_;
    };

public:
    /**
     * @brief Constructs a tag and assigns it a unique Id.
     * @param name The string representation of the message type.
     */
    explicit IMessageTag(std::string_view name);

    virtual ~IMessageTag() = default;

    /** @brief Gets the unique Id of this message tag. */
    Id id() const;

    /** @brief Gets the string name of this message tag. */
    std::string_view name() const;

private:
    const Id id_;

    const std::string_view name_;
};


/**
 * @brief Templated tag used to associate a message name with a specific payload type T.
 * @tparam T The payload type.
 */
template<typename T>
class MessageTag : public IMessageTag
{
public:
    explicit MessageTag(std::string_view name)
        : IMessageTag(name)
    {
    }

    ~MessageTag() = default;
};


namespace std
{
    /**
     * @brief Template specialization for std::hash to allow IMessageTag::Id in unordered containers.
     */
    template <>
    struct hash<IMessageTag::Id>
    {
        std::size_t operator()(const IMessageTag::Id& id) const noexcept
        {
            return std::hash<std::uint32_t>{}(id.get());
        }
    };
}
