#pragma once
#include <message_bus/MessageDispatcher.hpp>
#include <message_bus/types/MessageTaskP.hpp>


class MessageDispatcherProxy
{
public:
	//-----------------------------------------------------------------------------
	// Constructor/Destructor
	//-----------------------------------------------------------------------------

	MessageDispatcherProxy();

	virtual ~MessageDispatcherProxy() = default;


	//-----------------------------------------------------------------------------
	// Class operators
	//-----------------------------------------------------------------------------

	/**
	 * @brief Subscribes a class member function to a specific message tag.
	 * See 'MessageDispatcher::subscribe' for more details.
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
	 * See 'MessageDispatcher::subscribe' for more details.
	 */
	template<typename MessageType, typename Callable>
		requires std::derived_from<MessageType, Message>
	MessageSubscription<MessageType>& subscribe(
		const MessageTag<MessageType>& tag,
		Callable&& fn
	);

	/**
	 * @brief Removes a previously registered class member subscription.
	 * See 'MessageDispatcher::unsubscribe' for more details.
	 */
	template<typename MessageType, typename Class>
		requires std::derived_from<MessageType, Message>
	void unsubscribe(
		const MessageTag<MessageType>& tag,
		void (Class::* method)(const MessageType&),
		Class* obj
	);

protected:
	// Protected convenience publish method to be used by derived classes,
	// allowing them to send messages through the dispatcher.
	template<typename MessageType>
		requires std::derived_from<MessageType, Message>
	void publish(const MessageType& message);

protected:
	std::shared_ptr<MessageDispatcher> msgDispatcher_;
};

#include "MessageDispatcherProxy.inl"
