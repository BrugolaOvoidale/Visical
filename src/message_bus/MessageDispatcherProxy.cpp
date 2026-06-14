#include "MessageDispatcherProxy.hpp"


MessageDispatcherProxy::MessageDispatcherProxy()
{
	msgDispatcher_ = std::make_shared<MessageDispatcher>();
}
