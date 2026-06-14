#pragma once
#include <wx/frame.h>
#include <enum/EnumTraits.hpp>
#include "camera/CameraWidget.hpp"
#include "../BaseView.hpp"


// Forward declarations
class ParameterInfo;
class wxRadioButton;
class wxTextCtrl;
class wxCheckBox;
class BusyOverlay;
class CameraWidgetList;
class ParameterWidgetList;
class ParameterChangedEvent;


/**
 * @brief The main graphical window for interacting with system cameras.
 */
class CameraAssistantView : public BaseView, public wxFrame
{
public:
    //-----------------------------------------------------------------------------
    // Enums
    //-----------------------------------------------------------------------------

    /**
     * @brief High-level states controlling modal busy overlays and global window interactivity.
     */
    enum class UiState {
        IDLE,                         ///< Window is ready for input; interaction elements are active.
        ENUMERATION,                  ///< actively searching for interfaces; device layout panels are masked.
        LOADING_CAMERA_PARAMETERS     ///< Fetching internal features from the targeted camera instance.
    };

    /**
     * @brief Defines the origin of cameras list.
     */
    enum class Source
    {
        FILE,         ///< Parsing using local manifest schemas.
        ENUMERATION   ///< Live query probing hardware interface layers natively.
    };


    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    CameraAssistantView(
        const wxString& title,
        bool startShowed = false
    );

	~CameraAssistantView() override = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /**
     * @brief Shows the frame and brings it to the front.
     */
    void Open();

    /** @brief Sets the file path of the cameras list. */
    void SetCamerasConfigPath(const wxString& path);

    /**
     * @brief Shifts internal component state layouts and triggers contextual busy loading screens.
     * @param uiState Target layout context descriptor.
     */
    void SetUiState(UiState uiState);

    /** @brief Sets the origin of cameras list. */
    void SetCameraSource(Source source);

    /** @brief Returns the selected origin of cameras list. */
    Source GetCameraSource() const;

    /** @brief Enable cameras discovery on startup. */
    void EnableAutoDiscovery(bool enable = true);

    /** @brief Returns camera discovery on startup value */
    bool GetAutoDiscovery();

    /** @brief Sets possible camera behaviours upon discovery. */
    void SetDiscoveryBehaviours(const std::vector<wxString>& choices);

    /** @brief Sets the default camera behaviour upon discovery. */
    void SelectDiscoveryBehaviour(const wxString& choice);

    /** @brief Updates a widget with its camera interface. */
    void UpdateCamera(const std::shared_ptr<ICameraInfo>& camera);

    /**
     * @brief Applies the current transitional status of a camera.
     * @param cameraId Unique camera identifier.
     * @param camOp Transitional status.
     */
    void SetCameraOperation(
        const wxString& cameraId,
        CameraWidget::Operation camOp
    );

    /** @brief Appends an individual camera. */
    void AppendCamera(
        const std::shared_ptr<ICameraInfo>& camera,
        const wxArrayString& camBehaviours
    );

    /** @brief Focuses selection focus on a specific camera. */
    void SelectCamera(const wxString& cameraId);

    /** @brief Gets selected camera, std::nullopt otherwise */
    std::optional<wxString> GetSelectedCamera() const;

    /** @brief Removes a specific camera. */
    void RemoveCamera(const wxString& cameraId);

    /** @brief Removes all cameras. */
    void RemoveAllCameras();

    /** @brief Populates the parameters categories. */
    void SetCategories(const std::vector<wxString>& categories);

    /** @brief Focuses selection on a specific category. */
    void SelectCategory(const wxString& categories);

    /** @brief Gets the selected catogery. */
    wxString GetSelectedCategory() const;

    /** @brief Populates the parameters visibilities. */
    void SetVisibilities(const std::vector<wxString>& visibilities);

    /** @brief Focuses selection on a specific visibility. */
    void SelectVisibility(const wxString& visibility);

    /** @brief Gets the selected visibility. */
    wxString GetSelectedVisibility() const;

    /** @brief Populates the parameters. */
    void SetParameters(const std::vector<std::shared_ptr<ParameterInfo>>& params);

    /** @brief Updates a widget with its parameter interface. */
    void UpdateParameter(const std::shared_ptr<ParameterInfo>& param);

    /** @brief Visually marks a parameter as dirty. */
    void MarkParameterAsDirty(const wxString& paramId, bool isDirty);

    /** @brief Removes all parameters. */
    void RemoveAllParameters();

    /** @brief Populates the camera interfaces. */
    void SetInterfaces(const std::vector<wxString>& interfaces);

    /** @brief Set camera parameters persistence. */
    void SetParamsPersistence(bool arePersistent);

private:
    wxPanel* CreateConnectionSourcePanel(wxWindow* parent);

    wxPanel* CreateCamerasPanel(wxWindow* parent);

    void DiscoveryFromFileMode();

    void DiscoveryFromEnumMode();

    void IdleMode();

    void EnumerationMode();

    void LoadingCameraParametersMode();


    //-----------------------------------------------------------------------------
    // Events
    //-----------------------------------------------------------------------------

    void OnLoadSettings(wxCommandEvent& event);

    void OnSaveSettings(wxCommandEvent& event);

    void OnSaveSettingsAs(wxCommandEvent& event);

    void OnChangeDiscoveryBehaviour(wxCommandEvent& event);

    void OnDiscoverySourceFromEnumeration(wxCommandEvent& event);

    void OnDiscoverySourceFromFile(wxCommandEvent& event);

    void OnAutoDiscovery(wxCommandEvent& event);

    void OnDiscoverInterface(wxCommandEvent& event);
    
    void OnLoadCamerasFromFile(wxCommandEvent& event);

    void OnSaveCamerasToFile(wxCommandEvent& event);

    void OnChangeCategory(wxCommandEvent& event);

    void OnChangeVisibility(wxCommandEvent& event);

    void OnChangeParamsPersitence(wxCommandEvent& event);

    void OnClose(wxCloseEvent& event);

private:
    Source m_cameraSrc;

    wxMenuItem* m_loadAppSettings;

    wxMenuItem* m_saveAppSettings;

    wxMenuItem* m_saveAppSettingsAs;

    wxCheckBox* m_autoDiscovery;

    wxChoice* m_discoveryBehaviour;

    // Connection source controls
    wxRadioButton* m_rbFromEnumeration;

    wxRadioButton* m_rbFromFile;

    // Discovery controls
    wxButton* m_btnAutoDiscovery;

    wxChoice* m_choiceInterfaces;

    wxButton* m_btnDiscover;

    // File controls
    wxStaticText* m_lblFile;

    wxTextCtrl* m_txtFilePath;

    wxButton* m_btnBrowse;

    //
    BusyOverlay* m_busyOverlayCam;

    CameraWidgetList* m_cameraList;

    wxPanel* m_paramsContentPanel;

    wxChoice* m_categories;

    wxChoice* m_visibilities;

    wxCheckBox* m_persistentEdits;

    BusyOverlay* m_busyOverlayCamParams;

    ParameterWidgetList* m_cameraParameterList;
};


template <>
struct EnumTraits<CameraAssistantView::Source> {
    static constexpr std::array<std::pair<CameraAssistantView::Source, const char*>, 2> values{ {
    {CameraAssistantView::Source::FILE, "file"},
    {CameraAssistantView::Source::ENUMERATION, "enumeration"},
    } };
};