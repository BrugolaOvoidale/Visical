#pragma once


/**
 * @brief Snaphot of the current physical and operational status of a camera.
 * 
 * This class encapsulates three levels of camera status:
 * 
 * 1. Presence: Is the hardware physically reachable?
 * 
 * 2. Connection: Is the driver/software layer attached to the hardware?
 * 
 * 3. Activity: Is the camera currently streaming data?
 */
class CameraState
{
public:
    //-----------------------------------------------------------------------------
    // Enums
    //-----------------------------------------------------------------------------

    /** @brief Physical presence of the device. */
    enum class Presence
    {
        UNKNOWN = -1,       ///< State not yet determined.
        NOT_DETECTED,       ///< Camera is not physically found on the bus/network.
        DETECTED            ///< Camera is physically present.
    };

    /** @brief Software connection status. */
    enum class Connection
    {
        UNKNOWN = -1,       ///< State not yet determined.
        DISCONNECTED,       ///< Detected, but not opened by a driver.
        CONNECTED           ///< Successfully initialized and ready for commands.
    };

    /** @brief Operational streaming status. */
    enum class Activity
    {
        UNKNOWN = -1,       ///< State not yet determined.
        IDLE,               ///< Connected, but not currently acquiring frames.
        GRABBING            ///< Actively acquiring and transmitting frames.
    };


    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    /** @brief Default constructor initializing all states to UNKNOWN. */
    CameraState() = default;

    /**
     * @brief Explicit constructor to set a full state snapshot.
     * @param p Physical presence status.
     * @param c Connection status.
     * @param a Current activity status.
     */
    CameraState(Presence p, Connection c, Activity a)
        : presence_(p), connection_(c), activity_(a)
    {}


    //-----------------------------------------------------------------------------
    // Logical operators
    //-----------------------------------------------------------------------------

    bool operator==(const CameraState& other) const = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /** @brief Gets the current physical presence status. */
    Presence presence() const { return presence_; }

    /** @brief Gets the current software connection status. */
    Connection connection() const { return connection_; }

    /** @brief Gets the current operational activity status. */
    Activity activity() const { return activity_; }

private:
    // Physical state
    Presence presence_{ Presence::UNKNOWN };

    // Driver state
    Connection connection_{ Connection::UNKNOWN };

	// Operational state
    Activity activity_{ Activity::UNKNOWN };
};
