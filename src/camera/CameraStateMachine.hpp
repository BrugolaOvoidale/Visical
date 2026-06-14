#pragma once
#include "CameraState.hpp"


/**
 * @class CameraStateMachine
 * @brief An immutable state machine tracker for managing camera life cycles.
 *
 * This class uses a value-based state pattern where transitions do not mutate 
 * the existing object, but instead return a new CameraStateMachine instance representing 
 * the target state.
 * 
 * Typical lifecycle flow: NotDetected -> Detected -> Connected -> (Idle <-> Grabbing) -> Disconnected.
 */
class CameraStateMachine
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    CameraStateMachine() = default;

    ~CameraStateMachine() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------
    
    /**
     * @brief Constructs a new CameraStateMachine in a "not detected" state.
     * @returns A CameraStateMachine.
     */
    [[nodiscard]] CameraStateMachine notDetected() const;

    /**
     * @brief Constructs a new CameraStateMachine in a "detected" state.
     * @returns A CameraStateMachine.
     */
    [[nodiscard]] CameraStateMachine detected() const;

    /**
     * @brief Constructs a new CameraStateMachine in a "connected" state.
     * @returns A CameraStateMachine.
     * @throws std::logic_error if camera is not detected.
     */
    [[nodiscard]] CameraStateMachine connected() const;

    /**
     * @brief Constructs a new CameraStateMachine in a "disconnected" state.
     * @returns A CameraStateMachine.
     * @throws std::logic_error if camera is not connected and its not in idle.
     */
    [[nodiscard]] CameraStateMachine disconnected() const;

    /**
     * @brief Constructs a new CameraStateMachine in a "grabbing" state.
     * @returns A CameraStateMachine.
     * @throws std::logic_error if camera is not connected.
     */
    [[nodiscard]] CameraStateMachine grabbing() const;

    /**
     * @brief Constructs a new CameraStateMachine in an "idle" state.
     * @returns A CameraStateMachine.
     * @throws std::logic_error if camera is not connected.
     */
    [[nodiscard]] CameraStateMachine idle() const;

    /** @brief Returns the current camera state. */
    CameraState state() const;

private:
    CameraStateMachine(CameraState::Presence p, CameraState::Connection c, CameraState::Activity a);

private:
    CameraState state_;
};