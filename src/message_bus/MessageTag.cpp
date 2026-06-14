#include "MessageTag.hpp"
#include <atomic>


/**
 * @class MessageIdAllocator
 * @brief Thread-safe utility to generate unique identifiers.
 */
class MessageIdAllocator
{
public:
    /** @brief Allocates a new unique 32-bit unsigned integer. */
    static std::uint32_t allocate()
    {
        static std::atomic<std::uint32_t> counter{ 0 };
        return ++counter;
    }
};


/////////////////////////////////////////////////////////////////


IMessageTag::IMessageTag(std::string_view name)
    : id_(MessageIdAllocator::allocate()),
    name_(name)
{
}

IMessageTag::Id IMessageTag::id() const
{
    return id_;
}

std::string_view IMessageTag::name() const
{
    return name_;
}