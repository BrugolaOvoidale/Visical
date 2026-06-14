#pragma once
#include <memory>
#include <unordered_map>
#include <thread/Thread.hpp>
#include "MessageBus.hpp"
#include "MessageTag.hpp"


/**
 * @brief Orchestrates message routing between publishers and subscribers.
 * 
 * The MessageDispatcher manages multiple type-specific message buses. It allows
 * for asynchronous message delivery via an internal worker thread and provides
 * a centralized interface for subscription management.
 */
class MessageDispatcher
{
public:
	//-----------------------------------------------------------------------------
	// Constructor/Destructor
	//-----------------------------------------------------------------------------

	/** @brief Initializes the dispatcher and starts the internal worker thread. */
	MessageDispatcher();

	~MessageDispatcher() = default;


	//-----------------------------------------------------------------------------
	// Class operators
	//-----------------------------------------------------------------------------

	/**
	 * @brief Stops the internal worker thread and clears all message buses.
	 * After calling shutdown, no further messages will be processed.
	 */
	void shutdown();

	/**
	 * @brief Subscribes a class member function to a specific message tag.
	 * @tparam MessageType The type of the message to listen for.
	 * @tparam Class The class type containing the method.
	 * @param tag The unique tag identifying the message channel.
	 * @param method Pointer to the member function (callback).
	 * @param obj Pointer to the instance of the class.
	 * @return Reference to the created MessageSubscription.
	 */
	template<typename MessageType, typename Class>
		requires std::derived_from<MessageType, Message>
	MessageSubscription<MessageType>& subscribe(
		const MessageTag<MessageType>& tag,
		void (Class::* method)(const MessageType&),
		Class* obj
	);

	/**
	 * @brief Subscribes a generic callable (lambda, functor) to a specific message tag.
	 * @tparam MessageType The type of the message to listen for.
	 * @tparam Callable The type of the callable object.
	 * @param tag The unique tag identifying the message channel.
	 * @param fn The callable object to execute upon message arrival.
	 * @return Reference to the created MessageSubscription.
	 */
	template<typename MessageType, typename Callable>
		requires std::derived_from<MessageType, Message>
	MessageSubscription<MessageType>& subscribe(
		const MessageTag<MessageType>& tag,
		Callable&& fn
	);

	/**
	 * @brief Removes a previously registered class member subscription.
	 * @param tag The tag associated with the subscription.
	 * @param method The member function pointer used during subscription.
	 * @param obj The class instance pointer used during subscription.
	 */
	template<typename MessageType, typename Class>
		requires std::derived_from<MessageType, Message>
	void unsubscribe(
		const MessageTag<MessageType>& tag,
		void (Class::* method)(const MessageType&),
		Class* obj
	);

	/**
	 * @brief Asynchronously publishes a message to all registered subscribers.
	 * @tparam MessageType The type of message to broadcast.
	 * @param message The message data to be sent.
	 */
	template<typename MessageType>
		requires std::derived_from<MessageType, Message>
	void publish(const MessageType& message);

private:
	// Retrieves or creates the type-specific MessageBus for a given tag.
	template<typename MessageType>
		requires std::derived_from<MessageType, Message>
	std::shared_ptr<MessageBus<MessageType>> getBus(const IMessageTag& tag);

private:
	// Background thread for message processing.
	Thread worker_;

	// Synchronizes access to the buses_ map.
	std::mutex mutex_;

	// Map of message tags to their corresponding type-erased message buses.
	std::unordered_map<IMessageTag::Id, std::shared_ptr<IMessageBus>> buses_;
};

#include "MessageDispatcher.inl"
