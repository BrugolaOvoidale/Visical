#pragma once
#include <concepts>
#include <functional>
#include <shared_mutex>
#include <optional>


// Forward declarations
class Message;


/**
 * @brief A subscriber-side filter builder for the MessageBus system.
 * 
 * This class uses the Builder Pattern to allow users to attach specific predicates
 * to a subscription. Only messages that pass all filters (i.e., the predicate returns true)
 * will be delivered to the callback. This class is thread-safe for both filter updates and evaluation.
 * 
 * @tparam MessageType Must be a class derived from Message.
 */
template<typename MessageType>
    requires std::derived_from<MessageType, Message>
class MessageSubscription
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    MessageSubscription() = default;

    ~MessageSubscription() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /**
     * @brief Attaches a custom predicate to filter incoming messages.
     * 
     * The predicate is a function that takes a MessageType and returns a boolean.
     * If the predicate returns @c false, the message is ignored by this subscriber.
     * 
     * @param predicate A callable object (lambda, function pointer, etc.).
     * @return Reference to this MessageSubscription for method chaining.
     * 
     * * @par Example:
     * @code
     * subscription.filterBy([](const MyMsg& msg)
     * {
     *      return msg.value > 10;
     * });
     * @endcode
     */
    MessageSubscription<MessageType>& filterBy(std::function<bool(const MessageType&)> predicate);

    /**
     * @brief Removes the current custom filter.
     * @return Reference to this MessageSubscription for method chaining.
     */
    MessageSubscription<MessageType>& unsetCustomFilter();

    /**
     * @brief Retrieves the current filter if one exists.
     * @return An optional containing the filter function.
     */
    std::optional<std::function<bool(const MessageType&)>> getCustomFilter() const;

    /**
     * @brief Evaluates if a specific message should be accepted based on active filters.
     * @param event The message instance to evaluate.
     * @return True if no filter is set or if the event passes the filter; false otherwise.
     */
    bool shouldAccept(const MessageType& event) const;

private:
    // Protects access to the customFilter_
	mutable std::shared_mutex mutex_;

    // User-defined predicate function.
    std::optional<std::function<bool(const MessageType&)>> customFilter_;
};

#include "MessageSubscription.inl"
