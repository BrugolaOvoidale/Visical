#pragma once
#include <vector>
#include <memory>
#include <logger/Log.hpp>
#include "MessageContext.hpp"
#include "../SourceId.hpp"


// Forward declarations
class IMessageTag;


/**
 * @brief Represents a single, immutable notification within the system.
 *
 * A Message captures a specific occurrence, including its classification (Tag),
 * the origin (SourceId), and associated metadata or execution logs.
 * The timestamp is automatically captured at the moment of construction.
 */
class Message
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    /**
     * @brief Constructs a new Message.
     * @param tag The tag defining the type or category of the message.
     * @param id The identifier of the component that generated this message.
     * @param ctx Additional metadata context.
     * @param logs A collection of log entries associated with the event.
     */
    Message(
        const IMessageTag& tag,
        SourceId id = SourceId::none(),
        MessageContext ctx = {},
        std::vector<Log> logs = {}
    );

    /** @brief Returns a deep copy of the message and its metadata. */
    virtual std::shared_ptr<Message> clone() const;

    virtual ~Message() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /** @brief Returns the identifying tag of the message. */
    const IMessageTag& tag() const;

    /** @brief Returns the Id of the source that produced this message. */
    SourceId sourceId() const;

    /** @brief Returns the metadata context associated with this message. */
    const MessageContext& context() const;

    /** @brief Returns the system time when the message was created. */
    std::chrono::system_clock::time_point timestamp() const;

    /** @brief Returns the list of log entries attached to this message. */
    const std::vector<Log>& getLogs() const;

private:
    // Non-owning pointer to the message tag.
    const IMessageTag* tag_;

    // Originator of the message.
    SourceId sourceId_;

    // Metadata storage.
    MessageContext context_;

    // Collection of related log entries.
    std::vector<Log> logs_;

    // Creation timestamp.
    std::chrono::system_clock::time_point timestamp_;
};
