#include "UiTickProxy.hpp"


UiTickProxy::UiTickProxy(std::function<void(void)> cb)
    : callback_(std::move(cb)),
    timer_(this)
{
    Bind(wxEVT_TIMER, &UiTickProxy::OnTimer, this);
}

UiTickProxy::~UiTickProxy()
{
    timer_.Stop();
    
    Unbind(wxEVT_TIMER, &UiTickProxy::OnTimer, this);

    callback_ = nullptr;
}

//////////////////////////////////////////////////////

void UiTickProxy::Start(int intervalMs)
{
    timer_.Start(intervalMs);
}

bool UiTickProxy::IsRunning() const
{
    return timer_.IsRunning();
}

void UiTickProxy::Stop()
{
    timer_.Stop();
}

//////////////////////////////////////////////////////

void UiTickProxy::OnTimer(wxTimerEvent&)
{
    if (callback_)
        callback_();
}