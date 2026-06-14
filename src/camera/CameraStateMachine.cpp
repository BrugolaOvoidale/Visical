#include "CameraStateMachine.hpp"
#include <stdexcept>


CameraStateMachine::CameraStateMachine(CameraState::Presence p, CameraState::Connection c, CameraState::Activity a)
    : state_(p, c, a)
{
}

/// //////////////////////////////////////////////////////

CameraStateMachine CameraStateMachine::notDetected() const
{
    return { CameraState::Presence::NOT_DETECTED, CameraState::Connection::UNKNOWN, CameraState::Activity::UNKNOWN };
}

CameraStateMachine CameraStateMachine::detected() const
{
    return { CameraState::Presence::DETECTED, state_.connection(), state_.activity()};
}

CameraStateMachine CameraStateMachine::connected() const
{
    if (state_.presence() != CameraState::Presence::DETECTED)
    {
        throw std::logic_error("Invalid transition: connect()");
    }

    const CameraState::Activity currActivity = state_.activity();

    return { CameraState::Presence::DETECTED,
             CameraState::Connection::CONNECTED,
             currActivity == CameraState::Activity::UNKNOWN ? CameraState::Activity::IDLE : currActivity };
}

CameraStateMachine CameraStateMachine::disconnected() const
{
    if (state_.connection() != CameraState::Connection::CONNECTED ||
        state_.activity() != CameraState::Activity::IDLE)
    {
        throw std::logic_error("Invalid transition: disconnect()");
    }

    return { CameraState::Presence::DETECTED,
             CameraState::Connection::DISCONNECTED,
             CameraState::Activity::UNKNOWN };
}

CameraStateMachine CameraStateMachine::grabbing() const
{
    if (state_.connection() != CameraState::Connection::CONNECTED)
    {
        throw std::logic_error("Invalid transition: grab()");
    }

    return { CameraState::Presence::DETECTED,
             CameraState::Connection::CONNECTED,
             CameraState::Activity::GRABBING };
}

CameraStateMachine CameraStateMachine::idle() const
{
    if (state_.connection() != CameraState::Connection::CONNECTED)
    {
        throw std::logic_error("Invalid transition: stopGrab()");
    }

    return { CameraState::Presence::DETECTED,
             CameraState::Connection::CONNECTED,
             CameraState::Activity::IDLE };
}

CameraState CameraStateMachine::state() const
{
    return state_;
}
