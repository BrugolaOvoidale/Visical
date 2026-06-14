#pragma once
#include <memory>
#include <wx/bitmap.h>
#include <gui_elements/base_widget/BaseWidget.hpp>
#include <camera/CameraState.hpp>


// Forward declarations
class ICameraInfo;
class wxBitmapButton;
class wxChoice;
class wxAnimationCtrl;


/**
 * @brief A graphic component handling the display for an individual camera.
 */
class CameraWidget : public BaseWidget
{
public:
    //-----------------------------------------------------------------------------
    // Enums
    //-----------------------------------------------------------------------------

    /**
     * @brief Represents the active, transient hardware task currently being performed.
     * Used primarily to drive "busy" states, disable buttons, or run animations.
     */
    enum class Operation
    {
        NONE,               ///< The camera is idle or executing no active asynchronous transition.
        CHECKING,           ///< Actively probing the hardware link.
        CONNECTING,         ///< Establishing a communication link with the device.
        DISCONNECTING,      ///< Gracefully severing communication links.
        STARTING_GRAB,      ///< Initiating the image acquisition cycle.
        STOPPING_GRAB       ///< Halting the image acquisition cycle.
    };


    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    /**
     * @brief Constructs a new Camera Widget.
     * @param parent Pointer to the parent window layer.
     * @param camera Shared handle to structural info describing this specific camera instance.
     * @param camBehaviours List of starting strings representing possible discovery behaviours.
     */
    CameraWidget(
        wxWindow* parent,
        const std::shared_ptr<ICameraInfo>& camera,
        const wxArrayString& camBehaviours
    );

    ~CameraWidget() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /**
     * @brief Refreshes structural properties from.
     * @param camera The updated camera specification reference.
     */
    void Update(const std::shared_ptr<ICameraInfo>& camera);

    /** @brief Retrieves the unique identification string of the tied camera. */
    const wxString& GetCameraId() const;

    /**
     * @brief Shifts the UI layout scheme to visually match a state.
     * @note This triggers an internal synchronous visual redraw of layout controls.
     * @param camState The target hardware state enum.
     */
    void SetCameraState(CameraState camState);

    /** @brief Retrieves the currently displayed structural hardware state flag. */
    CameraState GetCameraState() const;

    /**
     * @brief Updates UI locks and busy animations to show an operation is in progress.
     * @param op The active transient task flag.
     */
    void SetCameraOperation(Operation op);

    /** @brief Retrieves the current transient state task flag. */
    Operation GetCameraOperation() const;

    /**
     * @brief populates or overrides the internal options map for the behavior selection menu.
     * @param newBehaviours Array of names specifying modes or analytical configurations.
     */
    void SetBehaviours(const wxArrayString& newBehaviours);

    /**
     * @brief Forces the behavior choice drop-down menu to a specific value without throwing events.
     * @param selBhaviour The target behavior string text.
     * @return true if the option existed and was chosen, false otherwise.
     */
    bool SelectBehaviour(const wxString& selBehaviour);

    /** @brief Gets the selection text string current inside the choice drop-down component. */
    const wxString& GetSelectedBehaviour() const;

    /**
     * @brief Updates the label text displaying the hardware connection layer (e.g., USB3, GigE).
     * @param interfaceType Explicit string representation of connection bus type.
     */
    void SetInterfaceType(const wxString& interfaceType);

    /** @brief Returns the interface bus layout text tag. */
    const wxString& GetInterfaceType() const;

private:
    /** @brief Tracking definitions for custom child control IDs. */
    enum DelaySelect {
        ConnectBtn = wxID_HIGHEST + 1        ///< Explicit offset tracking to prevent overlapping collision domains.
    };

    wxPanel* CreateHeader() override;

    void InitializeImpl() override;

    void SetDescriptionImpl() override;

    void DoSetState();

    void DoSetOperation();

    void DoSetBehaviours();

    bool DoSelectBehaviour();

    void DoSetInterfaceType();

    void UnknownState();

    void BusyState(const wxString& statusTxtTag);

    void NotDetectedState();

    void DetectedState();

    void IdleState();

    void GrabbingState();


    //-----------------------------------------------------------------------------
    // Events
    //-----------------------------------------------------------------------------

    void OnSpecializedWidgetClick(wxMouseEvent& event) override;

    void OnCheckCamera(wxCommandEvent& event);

    void OnConnectDisconnectCamera(wxCommandEvent& event);

    void OnChangeBehaviour(wxCommandEvent& event);

private:
    // Unique identifier string for the managed camera.
    wxString m_cameraId;

    // Internal storage for behavior mode.
    wxArrayString m_camBehaviours;

    // Currently selected behavior name string.
    wxString m_currBehaviour;

    // Fallback or working copy of behavior modes.
    std::vector<wxString> m_behaviours;

    // Cache string for interface mode text display.
    wxString m_interfaceTypeStr;

    // Status animation display element.
    wxAnimationCtrl* m_status{ nullptr };

    // Text element displaying the camera Id.
    wxStaticText* m_cameraIdTxt{ nullptr };

    // Text label for human-readable state strings.
    wxStaticText* m_statusTxt{ nullptr };

    // Button trigger for manual diagnostic checks.
    wxBitmapButton* m_checkButton{ nullptr };

    // Visual asset icon for "Connect" operation.
    wxBitmap m_connectBitmap;

    // Visual asset icon for "Disconnect" operation.
    wxBitmap m_disconnectBitmap;

    // Tracks which icon state is actively rendering on button layout maps.
    bool m_isDisconnectBitmap{ false };

    // Context-sensitive functional connection toggle control.
    wxBitmapButton* m_connectButton{ nullptr };

    // Display component for interface classification tags.
    wxStaticText* m_interfaceType{ nullptr };

    // Dropdown selector component for processing modes.
    wxChoice* m_behaviourChoice{ nullptr };

    // Enum tracking steady-state device profile status.
    CameraState m_camState;

    // Enum tracking real-time async task actions.
    Operation m_camOp;
};
