#pragma once
#include "Message.hpp"


/**
 * @brief A message representing the state and logs of a simple
 * one shot event that carries a payload.
 */
template<typename T>
class MessageP : public Message
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    /**
     * @brief Constructs a new Message, carrying a payload.
     * @param tag The tag defining the type or category of the message.
     * @param T The payload carried by the message.
     * @param id The identifier of the component that generated this message.
     * @param ctx Additional metadata context.
     * @param logs A collection of log entries associated with the event.
     */
    MessageP(
        const IMessageTag& tag,
        T p,
        SourceId id = SourceId::none(),
        MessageContext ctx = {},
        std::vector<Log> logs = {}
    );

    ~MessageP() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /** @brief Set the payload. */
    void setPayload(T p);

    /** @brief Return the payload. */
    const T& getPayload() const;

private:
    T payload_;
};

#include "MessageP.inl"