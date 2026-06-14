#pragma once
#include "MessageDispatcher.hpp"


template<typename MessageType, typename Class>
	requires std::derived_from<MessageType, Message>
inline MessageSubscription<MessageType>& MessageDispatcher::subscribe(
	const MessageTag<MessageType>& tag,
	void(Class::* method)(const MessageType&),
    Class* obj)
{
	return getBus<MessageType>(tag)->subscribe(method, obj);
}

template<typename MessageType, typename Callable>
	requires std::derived_from<MessageType, Message>
inline MessageSubscription<MessageType>& MessageDispatcher::subscribe(
	const MessageTag<MessageType>& tag,
	Callable&& fn)
{
	return getBus<MessageType>(tag)->subscribe(std::forward<Callable>(fn));
}

template<typename MessageType, typename Class>
	requires std::derived_from<MessageType, Message>
inline void MessageDispatcher::unsubscribe(
	const MessageTag<MessageType>& tag,
	void(Class::* method)(const MessageType&),
	Class* obj)
{
	return getBus<MessageType>(tag)->unsubscribe(method, obj);
}

template<typename MessageType>
	requires std::derived_from<MessageType, Message>
inline void MessageDispatcher::publish(const MessageType& message)
{
	std::shared_ptr<MessageBus<MessageType>> bus = getBus<MessageType>(message.tag());

	worker_.enqueueTask([bus, message]() {
		bus->publish(message);
		});
}

///////////////////////////////////////////////////////////////////////

template<typename MessageType>
	requires std::derived_from<MessageType, Message>
inline std::shared_ptr<MessageBus<MessageType>> MessageDispatcher::getBus(const IMessageTag& tag)
{
	std::lock_guard lock(mutex_);

	IMessageTag::Id msgId = tag.id();

	auto it = buses_.find(msgId);
	if (it == buses_.end())
	{
		std::shared_ptr<MessageBus<MessageType>> bus = std::make_shared<MessageBus<MessageType>>();

		buses_[msgId] = bus;

		return bus;
	}

	return std::dynamic_pointer_cast<MessageBus<MessageType>>(buses_[msgId]);
}