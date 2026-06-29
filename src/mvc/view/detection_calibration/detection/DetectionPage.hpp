#pragma once
#include <enum/EnumTraits.hpp>
#include "ParameterLocation.hpp"
#include "../common/CalibrationStageView.hpp"


// Forward declarations
class wxRadioButton;
class wxAnimationCtrl;
class wxStaticText;
class wxChoice;
class ImagePreprocess;
class ParameterWidgetList;
class BoardEvent;


/**
 * @brief View responsible for managing the board detection calibration step.
 *
 * This panel provides user interfaces to acquire calibration patterns from either local image
 * files or live camera streams, handles regional/detection parameters, and displays real-time
 * feedback of calibration targets.
 */
class DetectionPage : public CalibrationStageView
{
public:
    //-----------------------------------------------------------------------------
    // Enums
    //-----------------------------------------------------------------------------

    /** @brief Specifies where calibration images are sourced from. */
    enum class ImageSource
    {
        FILE,           ///< Load pre-existing frames from disk.
        CAMERA          ///< Stream live frames from a connected camera device.
    };

    /** @brief Represents the internal state machine driving the UI element availability. */
    enum class UiState {
        IDLE,                ///< UI is waiting for user interaction.
        BUSY,                ///< Running generic heavy backend processing.
        START_LIVE,          ///< Trying to start Live.
        STOP_LIVE,           ///< Trying to stop Live.
        LIVE,                ///< Active camera streaming feed.
        BOARD_FROM_FILE,     ///< Loading and analyzing a static file.
        BOARD_UPDATE,        ///< Modifying single board information.
        BOARD_REEVALUATE,    ///< Recalculating detection on a single target.
        SEQUENCE_REEVALUATE, ///< Recalculating detection across a multi-frame dataset.
    };


    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    DetectionPage(
        wxWindow* parent,
        wxWindowID winid = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxTAB_TRAVERSAL | wxNO_BORDER,
        const wxString& name = wxASCII_STR(wxPanelNameStr)
    );

    ~DetectionPage() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /**
     * @brief Populates the parameter panels within the view.
     * @param params Vector containing configurations blocks.
     * @param where Identifies which target sub-panel layout hosts these parameters.
     */
    void SetParameters(
        const std::vector<std::shared_ptr<ParameterInfo>>& params,
        DetectionView::ParameterLocation where
    );

    /** @brief Updates a specific parameters inside the view tracker. */
    void UpdateParameter(const std::shared_ptr<ParameterInfo>& param) override;

    /** @brief Flags a parameter visually to notify users that it has unsaved edits. */
    void MarkParameterAsDirty(
        const wxString& paramId,
        const wxString& categoryId,
        bool isDirty
    ) override;

    /** @brief Clears active device selections. */
    void UnselectCamera();

    /** @brief Sets the active device selection. */
    void SelectCamera(const wxString& camera);

    /** @brief Returns the identifier of the currently active camera. */
    wxString GetSelectedCamera() const;

    /** @brief Validates if a camera is actively selected. */
    bool IsCameraSelected() const;

    /** @brief Appends a camera. */
    void AddCamera(const wxString& cameraId);

    /** @brief Removes a camera. */
    void RemoveCamera(const wxString& cameraId);

    /** @brief Clears the drop-down device inventory completely. */
    void RemoveAllCameras();

    /**
     * @brief Transitions the page into a specific execution layout state.
     * 
     * Automatically activates/deactivates button configurations to match state safety requirements.
     */
    void SetUiState(UiState uiState);

    /** @brief Gets current execution state. */
    UiState GetUiState() const;

    /** @brief Sets the active operational configuration origins. */
    void SetImageSource(ImageSource src);

    /** @brief Returns active operational configuration origins. */
    ImageSource GetImageSource() const;

    /** @brief Sets Auto Capture value. */
    void SetAutoCapture(bool checked);

    /** @brief Forces selection highlight onto a specific board item. */
    void SelectBoard(std::uint32_t id) override;

private:
    wxNotebook* CreateNotebook();

    wxPanel* CreateSetupDetectionTab(wxWindow* parent);

    wxPanel* CreateDetectionSubTab(wxWindow* parent);

    wxPanel* CreateFeedPanel(wxWindow* parent);

    wxPanel* CreateImageDisplayPanel(wxWindow* parent);

    wxPanel* CreateDetectionPanel(wxWindow* parent);

    void DatasetUpdated() override;

    void UpdateAcceptDatasetBtn();

    void UpdateSaveImagesBtn();

    void UpdateRemoveBoardBtn();

    void UpdateRemoveAllBoardsBtn();

    void UpdateCameraFeedBtns();

    void ImgAcqAssistantUIMode();

    void LoadImagesUIMode();

    void IdleMode();

    void BusyMode();

    void StartLiveMode();

    void StopLiveMode();

    void LiveMode();

    void BoardFromFileMode();

    void BoardUpdateMode();

    void BoardReevaluateMode();

    void SequenceReevaluateMode();


    //-----------------------------------------------------------------------------
    // Events
    //-----------------------------------------------------------------------------

    void OnBoardClick(BoardEvent& event);

    void OnRemoveBoard(wxCommandEvent& event);

    void OnRemoveAllBoards(wxCommandEvent& event);

    void OnChangeCamera(wxCommandEvent& event);

    void OnImageSourceToFile(wxCommandEvent& event);

    void OnImageSourceToCamera(wxCommandEvent& event);

    void OnOpenCameraAssistant(wxCommandEvent& event);

    void OnLoadImage(wxCommandEvent& event);

    void OnSnap(wxCommandEvent& event);

    void OnLive(wxCommandEvent& event);

    void OnAutoCapture(wxCommandEvent& event);

    void OnSaveAllBoards(wxCommandEvent& event);

    void OnImagePreProcess(wxCommandEvent& event);

    void OnAcceptDataset(wxCommandEvent& event);

private:
    UiState m_uiState;

    DetectionPage::ImageSource m_imgSrc;

    wxRadioButton* m_imageFilesRadio;

    wxRadioButton* m_imgAcqAssistRadio;

    wxBitmapButton* m_cameraManager;

    wxPanel* m_imageDisplayPanel;

    wxPanel* m_feedPanel;

    wxAnimationCtrl* m_feedIcon;

    wxStaticText* m_feedTxt;

    wxButton* m_snapBtn;

    wxCheckBox* m_liveCheckBox;

    wxCheckBox* m_autoCaptureCheckBox;

    wxStaticText* m_deviceLabel;

    wxChoice* m_devicesList;

    wxBitmapButton* m_loadImgBtn;

    wxBitmapButton* m_saveImgBtn;

    wxBitmapButton* m_removeBoard;

    wxBitmapButton* m_removeAllBoards;

    ImagePreprocess* m_imagePreprocessFrame;

    wxBitmapButton* m_imgPreProcBtn;

    wxButton* m_acceptDatasetBtn;

    wxBitmapButton* m_loadModelParams;

    wxBitmapButton* m_saveModelParams;

    ParameterWidgetList* m_patternParamsList;

    ParameterWidgetList* m_geomParamsList;

    ParameterWidgetList* m_detParamsList;

    ParameterWidgetList* m_refineParamsList;
};


template <>
struct EnumTraits<DetectionPage::ImageSource> {
    static constexpr std::array<std::pair<DetectionPage::ImageSource, const char*>, 2> values{ {
    {DetectionPage::ImageSource::FILE, "file"},
    {DetectionPage::ImageSource::CAMERA, "camera"}
    } };
};