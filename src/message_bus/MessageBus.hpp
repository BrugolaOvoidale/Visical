#pragma once
#include <vector>
#include <mutex>
#include "MessageSubscription.hpp"
#include "MessageHandler.hpp"


// Forward declarations
class Message;
class IHandlerBase;


/**
 * @brief Base interface for Message Bus system.
 * 
 * Provides a polymorphic base to allow storing different specialized
 * MessageBus types in a common collection.
 */
class IMessageBus
{
public:
    IMessageBus() = default;

    virtual ~IMessageBus() = default;
};


/**
 * @brief A thread-safe, type-specific message dispatcher.
 * 
 * This class manages subscriptions and publications for a specific message type.
 * It uses a builder pattern for subscriptions, allowing users to attach filters.
 * 
 * @tparam MessageType The specific message class handled by this bus.
 * Must derive from Message.
 */
template<typename MessageType>
    requires std::derived_from<MessageType, Message>
class MessageBus : public IMessageBus
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    MessageBus() = default;
    
    ~MessageBus() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /**
     * @brief Subscribes a class member function to this message type.
     * 
     * This starts the subscription process. You can chain filter methods on the
     * returned object.
     * 
     * @tparam Class The type of the observing class.
     * @param method The member function pointer (callback).
     * @param obj The instance of the class on which to call the method.
     * @return MessageSubscription<MessageType>& A reference to the subscription builder.
     */
    template<typename Class>
    MessageSubscription<MessageType>& subscribe(
        void (Class::* method)(const MessageType&),
        Class* obj
    );

    /**
     * @brief Subscribes a free function, lambda, or functor to this message type.
     * 
     * @tparam Callable A functional type matching 'void(const MessageType&)'.
     * @param fn The callable object.
     * @return MessageSubscription<MessageType>& A reference to the subscription builder.
     */
    template<typename Callable>
    MessageSubscription<MessageType>& subscribe(Callable&& fn);

    /**
     * @brief Removes a class member function from the subscriber list.
     * 
     * @tparam Class The type of the observing class.
     * @param method The member function pointer used during subscription.
     * @param obj The instance of the class used during subscription.
     */
    template<typename Class>
    void unsubscribe(void (Class::* method)(const MessageType&), Class* obj);

    /**
     * @brief   Dispatches a message to all active and filtered subscribers.
     *          If a subscriber has filters, the callback only triggers if all filters pass.
     * 
     * Iterates through the internal subscriber list and executes callbacks.
     * @param message The message instance to broadcast.
     */
    void publish(const MessageType& message);

private:
    mutable std::mutex mutex_;

    std::vector<std::pair<std::shared_ptr<IHandlerBase>, std::shared_ptr<MessageSubscription<MessageType>>>> messages_;
};

#include "MessageBus.inl"
