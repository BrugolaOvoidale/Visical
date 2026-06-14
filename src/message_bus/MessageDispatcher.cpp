#include "MessageDispatcher.hpp"


MessageDispatcher::MessageDispatcher()
{
	worker_.start();
}

/////////////////////////////////////////


void MessageDispatcher::shutdown()
{
	worker_.shutdown();
}