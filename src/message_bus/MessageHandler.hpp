#pragma once
#include <functional>
#include <concepts>


// Forward declarations
class Message;


/**
 * @brief Abstract interface for a message handler.
 * 
 * Provides a polymorphic interface so that different types of handlers
 * can be stored in a single collection.
 */
class IHandlerBase
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    IHandlerBase() = default;

    virtual ~IHandlerBase() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /**
     * @brief Executes the callback associated with this handler.
     * @param message A pointer to the base Message object.
     */
    virtual void call(const Message* message) = 0;
};


/**
 * @brief Wraps a class member function to be used as a message callback.
 * @tparam MessageType The specific derived Message type this handler listens for.
 * @tparam Class The class type containing the member function.
 */
template<typename MessageType, typename Class>
    requires std::derived_from<MessageType, Message>
class MemberHandler : public IHandlerBase
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    /**
     * @brief Constructs a member function handler.
     * @param obj Pointer to the instance on which to call the method.
     * @param method Pointer to the member function (must take 'const MessageType&').
     */
    MemberHandler(
        Class* obj,
        void (Class::* method)(const MessageType&)
    );

    ~MemberHandler() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /**
     * @brief Casts the generic message and invokes the member function.
     * @param msg The message to process.
     */
    void call(const Message* msg) override;

    /**
     * @brief Checks if this handler matches a specific object-method pair.
     * Useful for unregistering specific handlers.
     */
    bool matches(
        Class* obj,
        void (Class::* method)(const MessageType&)
    ) const;

private:
    // Instance pointer.
    Class* object_;

    // Mthod pointer.
    void (Class::* method_)(const MessageType&);
};


/**
 * @brief Wraps a free function to be used as a message callback.
 * * @tparam MessageType The specific derived Message type this handler listens for.
 */
template<typename MessageType>
    requires std::derived_from<MessageType, Message>
class LambdaHandler : public IHandlerBase
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    /**
     * @brief Constructs a handler from a functional object.
     * @param fn A free function.
     */
    explicit LambdaHandler(std::function<void(const MessageType&)> fn);

    ~LambdaHandler() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------
    
    /**
     * @brief Casts the generic message and invokes the stored function.
     * @param msg The message to process.
     */
    void call(const Message* msg) override;

private:
    // Stored callback.
    std::function<void(const MessageType&)> fn_;
};

#include "MessageHandler.inl"
