#include "DetectionController.hpp"
#include <cinttypes>
#include <wx/app.h>
#include <wx/log.h>
#include <UtilityFunctions.hpp>
#include <camera_manager/CameraManager.hpp>
#include <calibration/Board.hpp>
#include <calibration/detector/PatternParametersRegistry.hpp>
#include <calibration/detector/ChessboardParametersRegistry.hpp>
#include <calibration/detector/CircleboardParametersRegistry.hpp>
#include <calibration/detector/ArucoParametersRegistry.hpp>
#include <calibration/detector/CharucoParametersRegistry.hpp>
#include <calibration/detector/AprilTagParametersRegistry.hpp>
#include <mvc/model/detection_calibration/SharedCameraIntrinsics.hpp>
#include <mvc/model/detection_calibration/detection/DetectionModelRegistry.hpp>
#include <mvc/model/detection_calibration/detection/DetectionModel.hpp>
#include <mvc/model/detection_calibration/detection/board/EvaluatedBoard.hpp>
#include <mvc/view/detection_calibration/detection/DetectionPage.hpp>
#include <mvc/view/detection_calibration/detection/DetectionPageEvents.hpp>
#include <mvc/view/detection_calibration/common/evaluation/EvaluationPanelEvents.hpp>
#include <mvc/view/detection_calibration/common/board/BoardWidgetDTO.hpp>
#include <gui_elements/image_panel/PixelEvent.hpp>
#include "DetectionSettings.hpp"
#include "DetectionUtility.hpp"
#include "DetectionViewState.hpp"
#include "DetectionResultView.hpp"
#include "../../UiTickProxy.hpp"
#include "../../BaseControllerLogger.hpp"


/////////////////////////////////////////////////////

static const wxString SNAP_TAG{ "SNAP" };
static const wxString LIVE_TAG{ "LIVE" };

static const std::string METADATA_IMAGE_TIMESTAMP{ "Image timestamp" };

static wxString FormatTimePoint(std::chrono::system_clock::time_point tp)
{
	auto s = time_point_cast<std::chrono::seconds>(tp);
	auto ms = duration_cast<std::chrono::milliseconds>(tp - s).count();

	wxDateTime dt((time_t)std::chrono::system_clock::to_time_t(s));

	return wxString::Format("%s.%03lld",
		dt.FormatISOCombined(' '),
		ms
	);
}

/////////////////////////////////////////////////////

DetectionController::DetectionController(
    DetectionPage* view,
    const std::shared_ptr<CameraManager>& cameraManager,
    const std::shared_ptr<SharedCameraIntrinsics>& camIntrinsics,
    const std::shared_ptr<BaseControllerLogger>& logger)
    : CalibrationStageController(
        DetectionModel::create(cameraManager, camIntrinsics, logger),
        view,
        std::make_shared<DetectionSettings>(),
        std::make_shared<DetectionUtility>(),
        logger)
{
	model_ = std::static_pointer_cast<DetectionModel>(model());

    view_ = view;

	settings_ = std::static_pointer_cast<DetectionSettings>(settings());

	utils_ = std::static_pointer_cast<DetectionUtility>(utils());

    viewState_ = std::make_unique<DetectionViewState>();

    DetectionController::init(cameraManager);

    CalibrationStageController::init();

    ReloadParameters();
}

DetectionController::~DetectionController()
{
    DetectionController::shutdown();
}

/////////////////////////////////////////////////////

void DetectionController::shutdown()
{
    model_->unsubscribe(MSG_BOARD_FROM_IMAGE, &DetectionController::onBoardDetectionFromImage, this);

    model_->unsubscribe(MSG_BOARD_FROM_SNAP, &DetectionController::onBoardDetectionFromSnap, this);

    model_->unsubscribe(MSG_BOARD_FROM_LIVE, &DetectionController::onBoardFromLive, this);

    model_->unsubscribe(MSG_BOARD_REDETECTION, &DetectionController::onBoardReDetection, this);

    model_->unsubscribe(MSG_BOARD_REEVALUATION, &DetectionController::onBoardReEvaluation, this);

    model_->unsubscribe(MSG_BOARD_STORED, &DetectionController::onBoardStored, this);

    model_->unsubscribe(MSG_BOARD_UPDATE, &DetectionController::onBoardUpdate, this);

    model_->unsubscribe(MSG_BOARD_SEQUENCE_REEVALUATED, &DetectionController::onBoardSequenceReEvaluated, this);

    model_->unsubscribe(MSG_PATTERN_TYPE_CHANGED, &DetectionController::onPatternTypeChanged, this);

    model_->shutdown();

    uiTickProxy_->Stop();
}

std::vector<std::shared_ptr<Board>> DetectionController::getDetectedBoards() const
{
    std::vector<std::shared_ptr<EvaluatedBoard>> evalBoards = model_->getDetectedBoards();

    std::vector<std::shared_ptr<Board>> boards;
    boards.reserve(evalBoards.size());

    for (const auto& e : evalBoards) boards.push_back(e->object());

    return boards;
}

void DetectionController::notifyCameraConnection(const std::string& cameraId)
{
    view_->AddCamera(cameraId);
}

void DetectionController::notifyCameraDisconnection(const std::string& cameraId)
{
    // TODO: clearing...

    view_->RemoveCamera(cameraId);
}

/////////////////////////////////////////////////////

void DetectionController::init(const std::shared_ptr<CameraManager>& cameraManager)
{
    // UI throttle timer
    uiTickProxy_ = std::make_unique<UiTickProxy>(
        [this]() { OnUiTick(); }
    );

    // Model
    model_->subscribe(MSG_BOARD_FROM_IMAGE, &DetectionController::onBoardDetectionFromImage, this);

    model_->subscribe(MSG_BOARD_FROM_SNAP, &DetectionController::onBoardDetectionFromSnap, this);

    model_->subscribe(MSG_BOARD_FROM_LIVE, &DetectionController::onBoardFromLive, this);

    model_->subscribe(MSG_BOARD_REDETECTION, &DetectionController::onBoardReDetection, this);

    model_->subscribe(MSG_BOARD_REEVALUATION, &DetectionController::onBoardReEvaluation, this);

    model_->subscribe(MSG_BOARD_STORED, &DetectionController::onBoardStored, this);

    model_->subscribe(MSG_BOARD_UPDATE, &DetectionController::onBoardUpdate, this);

    model_->subscribe(MSG_BOARD_SEQUENCE_REEVALUATED, &DetectionController::onBoardSequenceReEvaluated, this);

    model_->subscribe(MSG_PATTERN_TYPE_CHANGED, &DetectionController::onPatternTypeChanged, this);

    model_->subscribe(MSG_SESSION_CLEARED, &DetectionController::onSessionCleared, this);

    
    // View
    view_->Bind(GUI_LOAD_IMAGE, &DetectionController::OnLoadImage, this);
    view_->Bind(GUI_SAVE_IMAGE, &DetectionController::OnSaveImage, this);

    view_->Bind(GUI_SNAP_CAMERA, &DetectionController::OnSnap, this);
    view_->Bind(GUI_LIVE_CAMERA, &DetectionController::OnLiveCamera, this);

    view_->Bind(GUI_DET_REMOVE_BOARD, &DetectionController::OnRemoveBoard, this);
    view_->Bind(GUI_DET_REMOVE_ALL_BOARDS, &DetectionController::OnRemoveAllBoards, this);

    view_->Bind(GUI_BOARD_SHOW_DEB_RESULT, &DetectionController::OnShowPerBoardDebugResult, this);
    view_->Bind(GUI_SEQUENCE_SHOW_DEB_RESULT, &DetectionController::OnShowPerSequenceDebugResult, this);

    view_->Bind(GUI_CTRL_HOVER_PIXEL, &DetectionController::OnShowPixelTooltip, this);

    doLoadSettingsImpl();
}

void DetectionController::UpdateBoard(
    std::uint32_t id,
    const std::shared_ptr<DetectionResult>& boardRes)
{
    if (id == 0)
        return;

    wxString src;
    switch (boardRes->source())
    {
        case DetectionResult::Source::FILE:
            src = std::static_pointer_cast<DetectionResultFile>(boardRes)->imagePath();

            break;

        case DetectionResult::Source::CAMERA_SNAPSHOT:
            src = SNAP_TAG;

            break;

        case DetectionResult::Source::CAMERA_LIVE:
            src = LIVE_TAG;

            break;

        default:
            src = "UNDEFINED";

            break;
    }

    const wxString timestampValue = FormatTimePoint(boardRes->evaluatedBoard()->object()->image().timepoint());

    std::unordered_map<wxString, wxString> metadata;
    metadata.emplace(METADATA_IMAGE_TIMESTAMP, timestampValue);


    BoardWidgetDTO::BoardStatus err;
    switch (boardRes->evaluatedBoard()->object()->errorCode())
    {
        case Board::ErrorCode::NONE:
            err = BoardWidgetDTO::BoardStatus::DETECTED;

            break;

        case Board::ErrorCode::NOT_DETECTED:
            err = BoardWidgetDTO::BoardStatus::NOT_DETECTED;

            break;

        case Board::ErrorCode::POSE_NOT_SOLVED:
            err = BoardWidgetDTO::BoardStatus::POSE_NOT_SOLVED;

            break;
    }

    BoardWidgetDTO::EvaluationStatus evalStatus;
    switch (boardRes->evaluatedBoard()->status())
    {
        case EvaluatedBoard::Status::NOT_EVALUATED:
            evalStatus = BoardWidgetDTO::EvaluationStatus::NOT_EVALUATED;

            break;

        case EvaluatedBoard::Status::GOOD:
            evalStatus = BoardWidgetDTO::EvaluationStatus::GOOD;

            break;

        case EvaluatedBoard::Status::HAS_ISSUES:
            evalStatus = BoardWidgetDTO::EvaluationStatus::HAS_ISSUES;

            break;
    }


    BoardWidgetDTO boardDTO(
        id,
        src,
        err,
        evalStatus,
        metadata
    );

    const bool updateSuccess = view_->UpdateBoard(boardDTO);

    if (!updateSuccess)
    {
        view_->AppendBoard(boardDTO);
    }
}

void DetectionController::addDetectionTask()
{
    ++tasks_;

    requestStartTimer();
}

void DetectionController::completeDetectionTask()
{
    if (tasks_.load() > 0)
        --tasks_;
}

void DetectionController::requestStartTimer()
{
    if (!uiTickProxy_->IsRunning())
        uiTickProxy_->Start(15); // 15ms -> ~60 FPS UI
}

bool DetectionController::canIdle()
{
    // Drain tasks and high priority before stopping timer
    return  tasks_.load() == 0 &&
        utils_->getRunningCookTasks() <= 0 &&
        utils_->highPriorityEmpty();
}

void DetectionController::OnUiTick()
{
    if (canIdle())
    {
        uiTickProxy_->Stop();
        view_->SetUiState(DetectionPage::UiState::IDLE);
        return;
    }

    if (model_->isLiveSession())
    {
        utils_->cook({ DetectionResultMap::Id(0), model_->getLatestLiveDetectionResult() });
    }

    std::optional<DetectionUtility::Mail> res = utils_->takeHighPriority();
    if (!res.has_value())
    {
        res = utils_->takeLatest();
    }

    if (!res.has_value())
        return;

    // Single Board
    const std::shared_ptr<DetectionResultView>& boardResView = res->detectionResultView();

    if (boardResView)
    {
        const DetectionResultMap::Entry& storeRes = boardResView->boardResultEntry();
        const std::shared_ptr<DetectionResult>& boardRes = storeRes.second;
        const std::shared_ptr<EvaluatedBoard>& evalBoard = boardRes->evaluatedBoard();
        DetectionResultMap::Id storeId = storeRes.first;

        switch (boardRes->source())
        {
        case DetectionResult::Source::CAMERA_SNAPSHOT:
        case DetectionResult::Source::FILE:
        {
            UpdateBoard(storeId.get(), boardRes);

            if (!view_->IsBoardSelected() || view_->GetSelectedBoard().value() == storeId.get())
            {
                UpdatePluginResults(evalBoard->assessments());
            }

            break;
        }

        case DetectionResult::Source::CAMERA_LIVE:
            viewState_->SetCurrentLiveFrame(boardRes->id());

            if (!view_->IsBoardSelected())
            {
                UpdatePluginResults(evalBoard->assessments());
            }

            break;
        }

        if (!view_->IsBoardSelected())
            viewState_->SetCurrentRenderedBoard(boardRes->id());
    }

    // Board sequence
    const std::shared_ptr<EvaluatedBoardSequence>& boardSeqRes = res->evaluatedSequence();

    if (boardSeqRes)
    {
        UpdatePluginResults(boardSeqRes->assessments());
    }


    // Image display

    // If the "Show plugin debug" is checked, we want to render it
    if (view_->IsShowDebPluginSelected())
    {
        const auto [wxPluginId, pluginLocation] = view_->GetSelectedDebPlugin().value();
        const std::string pluginId = wxPluginId.ToStdString();

        if (pluginLocation == EvaluationPanel::PluginLocation::PER_BOARD)
        {
            if (boardResView) // if there is a board in the payload
            {
                const DetectionResultMap::Entry& storeRes = boardResView->boardResultEntry();
                const std::shared_ptr<DetectionResult>& boardRes = storeRes.second;

                auto updateDisplayTask = [&]()
                    {
                        std::optional<PluginDebugResultView> convRes = utils_->buildPluginDebugResult(pluginId, boardRes->evaluatedBoard());
                        if (convRes.has_value())    // plugin deb converted, render it
                        {
                            const PluginDebugResultView& deb = convRes.value();

                            view_->UpdateImageDisplay(deb.debugImage());
                        }
                        else  // utilsLayer has no builder for said plugin...
                        {
                            // ...fallback to displaying selected board image
                            view_->UpdateImageDisplay(
                                utils_->convertBoardImageToWx(boardRes->evaluatedBoard()->object())
                            );
                        }
                    };

                if (view_->IsBoardSelected())
                {
                    if (view_->GetSelectedBoard().value() == storeRes.first)
                    {
                        updateDisplayTask();
                    }
                }
                else
                {
                    updateDisplayTask();
                }
            }
        }
        else if (pluginLocation == EvaluationPanel::PluginLocation::PER_SEQUENCE)
        {
            const std::shared_ptr<EvaluatedBoardSequence>& evalSequence = res->evaluatedSequence();
            // Per-sequence
            if (evalSequence)
            {
                std::optional<PluginDebugResultView> convRes = utils_->buildPluginDebugResult(pluginId, evalSequence);
                if (convRes.has_value())    // plugin deb converted, render it
                {
                    const PluginDebugResultView& deb = convRes.value();

                    view_->UpdateImageDisplay(
                        deb.debugImage()
                    );
                }
                else  // utilsLayer has no builder for said plugin...
                {
                    // ...fallback to displaying selected board image
                    if (boardResView)
                    {
                        view_->UpdateImageDisplay(
                            utils_->convertBoardImageToWx(
                                boardResView->boardResultEntry().second->evaluatedBoard()->object())
                        );
                    }
                    else // ...fallback clearing display, if no board was selected
                    {
                        view_->ClearImageDisplay();
                    }
                }
            }
        }
    }
    else // else we want to render the board image...
    {
        // ... only if there is a board in payload, 
        // AND no board is selected OR the board in payload is the one selected
        if (boardResView)
        {
            auto updateDisplayTask = [&]()
                {
                    view_->UpdateImageDisplay(boardResView->wxBoardImage());
                };

            if (view_->IsBoardSelected())
            {
                if (view_->GetSelectedBoard().value() == boardResView->boardResultEntry().first)
                {
                    updateDisplayTask();
                }
            }
            else
            {
                updateDisplayTask();
            }
        }
    }
}

//
void DetectionController::doLoadSettingsImpl()
{    
    TaskResultP<DetectionSettings::Loaded> loadRes = settings_->loadSettings();

    UpdateLogs(loadRes.getLogs());

    if (loadRes.isSuccess())
    {
        DetectionSettings::Loaded payload = loadRes.takePayload();

        DetectionPage::ImageSource imgSrc = payload.imgSrc;

        view_->SetImageSource(imgSrc);

        view_->DrawBoard(payload.drawBoard);
        view_->DrawMarks(payload.drawMarks);
        view_->DrawWCS(payload.drawWCS);

        utils_->drawBoard(payload.drawBoard);
        utils_->drawMarks(payload.drawMarks);
        utils_->drawWCS(payload.drawWCS);
    }
}

//
void DetectionController::doSaveSettingsImpl()
{
    TaskResult res = settings_->saveSettings(
        view_->GetImageSource(),
        view_->IsDrawBoardEnabled(),
        view_->IsDrawMarksEnabled(),
        view_->IsDrawWCSEnabled()
    );

    UpdateLogs(res.takeLogs());
}

//
void DetectionController::SelectBoard(const BoardEvent& event)
{
    if (!view_->IsBoardSelected())
    {
        bool shouldClearDisplay = true;

        if (view_->IsShowDebPluginSelected())
        {
            const auto [_, pluginLocation] = view_->GetSelectedDebPlugin().value();

            if (pluginLocation == EvaluationPanel::PluginLocation::PER_SEQUENCE)
            {
                shouldClearDisplay = false;
            }
        }

        if (shouldClearDisplay)
            view_->ClearImageDisplay();

        view_->ClearAllPlugins(EvaluationPanel::PluginLocation::PER_BOARD);

        viewState_->UnsetCurrentRenderedBoard();

        return;
    }

    const std::uint32_t boardId = event.GetBoardId();

    std::optional<std::shared_ptr<DetectionResult>> opt_boardRes = model_->getDetectionResult(DetectionResultMap::Id(boardId));
    if (!opt_boardRes.has_value())
    {
        UpdateLogs(
            Log("Board id " + wxString::Format("%" PRIu32, boardId) + " does not exists")
        );

        return;
    }

    const std::shared_ptr<DetectionResult>& boardRes = opt_boardRes.value();
    const std::shared_ptr<EvaluatedBoard>& evalBoard = boardRes->evaluatedBoard();

    if (!view_->IsShowDebPluginSelected())
    {
        view_->UpdateImageDisplay(
            utils_->convertBoardImageToWx(evalBoard->object())
        );
    }
    else
    {
        const auto [wxPluginId, pluginLocation] = view_->GetSelectedDebPlugin().value();

        if (pluginLocation == EvaluationPanel::PluginLocation::PER_BOARD)
        {
            std::optional<PluginDebugResultView> convRes = utils_->buildPluginDebugResult(wxPluginId.ToStdString(), evalBoard);
            if (convRes.has_value())    // plugin deb converted, render it
            {
                const PluginDebugResultView& deb = convRes.value();

                view_->UpdateImageDisplay(deb.debugImage());
            }
            else  // utilsLayer has no builder for said plugin...
            {
                // ...fallback to displaying selected board image
                view_->UpdateImageDisplay(
                    utils_->convertBoardImageToWx(evalBoard->object())
                );
            }
        }
    }

    UpdatePluginResults(evalBoard->assessments());

    viewState_->SetCurrentRenderedBoard(boardRes->id());
}

//
void DetectionController::RemoveBoard(const BoardEvent& event)
{
    const std::uint32_t boardId = event.GetBoardId();

    view_->RemoveBoard(boardId);

    DetectionResultMap::Id resId(boardId);

    std::optional<DetectionResultMap::Id> newIdx = model_->removeDetectionResults(resId);

    if (!newIdx.has_value())
    {
        view_->ClearImageDisplay();
        view_->ClearAllPlugins(EvaluationPanel::PluginLocation::PER_BOARD);
        view_->UnselectBoard();
        viewState_->UnsetCurrentRenderedBoard();


        UpdateLogs(Log("Board id " + wxString::Format("%" PRIu64, boardId) + wxString(" doesn't exists in Model")));
        return;
    }

    DetectionResultMap::Id newIdx_value = newIdx.value();
    if (newIdx_value > 0)
    {
        std::shared_ptr<DetectionResult> boardRes = model_->getDetectionResult(newIdx_value).value();
        const std::shared_ptr<EvaluatedBoard>& evalBoard = boardRes->evaluatedBoard();
        const std::shared_ptr<Board>& board = evalBoard->object();

        // If the "Show plugin debug" is checked and the plugin is a Per-board plugin, we want to render it
        // Removing a board will trigger a sequence revaluation in the Model,
        // so plugin debugs per-sequence will be updated, and 'OnUiTick()' will take care of the rendering,
        // hence here we only need to check if pluginLocation is Per-board
        if (view_->IsShowDebPluginSelected())
        {
            const auto [wxPluginId, pluginLocation] = view_->GetSelectedDebPlugin().value();

            if (pluginLocation == EvaluationPanel::PluginLocation::PER_BOARD)
            {
                const std::string pluginId = wxPluginId.ToStdString();
                const std::vector<std::shared_ptr<PluginResult>>& evalAssessments = evalBoard->assessments();

                std::optional<PluginDebugResultView> convRes = utils_->buildPluginDebugResult(wxPluginId.ToStdString(), evalBoard);
                if (convRes.has_value())    // plugin deb converted, render it
                {
                    const PluginDebugResultView& deb = convRes.value();

                    view_->UpdateImageDisplay(deb.debugImage());
                }
                else  // utilsLayer has no builder for said plugin...
                {
                    // ...fallback to displaying selected board image
                    view_->UpdateImageDisplay(
                        utils_->convertBoardImageToWx(board)
                    );
                }
            }
        }
        else // else we want to render the next board image
        {
            view_->UpdateImageDisplay(
                utils_->convertBoardImageToWx(board)
            );
        }

        UpdatePluginResults(evalBoard->assessments());

        view_->SelectBoard(newIdx_value.get());

        viewState_->SetCurrentRenderedBoard(boardRes->id());
    }
    else
    {
        view_->ClearImageDisplay();
        view_->ClearAllPlugins(EvaluationPanel::PluginLocation::PER_BOARD);
        view_->UnselectBoard();
        viewState_->UnsetCurrentRenderedBoard();
    }
}

void DetectionController::OnRemoveBoard(BoardEvent& event)
{
    RemoveBoard(event);
}

//
void DetectionController::ClearSessionView()
{
    view_->RemoveAllBoards();
    view_->ClearAllPlugins(EvaluationPanel::PluginLocation::PER_BOARD);
    view_->ClearAllPlugins(EvaluationPanel::PluginLocation::PER_SEQUENCE);
    view_->UnselectBoard();
    viewState_->UnsetCurrentRenderedBoard();

    // If the "Show plugin debug" is checked and the plugin is a Per-sequence plugin, we want to render it
    if (view_->IsShowDebPluginSelected())
    {
        const auto [wxPluginId, pluginLocation] = view_->GetSelectedDebPlugin().value();

        if (pluginLocation == EvaluationPanel::PluginLocation::PER_SEQUENCE)
        {
            std::shared_ptr<EvaluatedBoardSequence> evalSeq = model_->getEvaluatedSequence();
            const std::vector<std::shared_ptr<PluginResult>>& evalAssessments = evalSeq->assessments();

            std::optional<PluginDebugResultView> convRes = utils_->buildPluginDebugResult(wxPluginId.ToStdString(), evalSeq);
            if (convRes.has_value())    // plugin deb converted, render it
            {
                const PluginDebugResultView& deb = convRes.value();

                view_->UpdateImageDisplay(deb.debugImage());
            }
            else  // utilsLayer has no builder for said plugin...
            {
                // ...fallback clearing display
                view_->ClearImageDisplay();
            }
        }
        else // the plugin was NOT a Per-sequence plugin, we want clear the display
        {
            view_->ClearImageDisplay();
        }
    }
    else // else clear display
    {
        view_->ClearImageDisplay();
    }
}

void DetectionController::RemoveAllBoards()
{
    model_->removeAllDetectionsResults();

    ClearSessionView();
}

void DetectionController::OnRemoveAllBoards(wxCommandEvent& event)
{
    RemoveAllBoards();
}

//
void DetectionController::SaveImage()
{
    std::map<DetectionResultMap::Id, std::shared_ptr<DetectionResult>> boards = model_->getAllDetectionResults();
    if (boards.empty())
        return;

    wxString message = "Save images";

    std::optional<wxString> wxFolderPath = view_->OpenDirDialog(message);
    if (!wxFolderPath.has_value())
        return; // user canceled save dialog

    const std::string folderPath = wxFolderPath.value().ToStdString();

    TaskResult res = DetectionUtility::saveImages(folderPath, boards);

    if (!res.isSuccess())
    {
        view_->ShowError("Failed to save images");
    }
    else
    {
        view_->ShowSuccess("Successfully saved images in " + folderPath);
    }

    UpdateLogs(res.getLogs());
}

void DetectionController::OnSaveImage(const wxCommandEvent& event)
{
    SaveImage();
}

//
void DetectionController::Snap()
{
    const wxString wxCameraID = view_->GetSelectedCamera();

    if (wxCameraID.IsEmpty()) return;

    // TODO: there is a difference between 'is live on' and 'is trying to live grab' !!!
    if (model_->isSessionOn() && model_->isLiveSession())
    {
        std::optional<DetectionResult::Id> opt_boardResId = viewState_->GetCurrentLiveFrame();
        if (opt_boardResId.has_value())
        {
            DetectionResult::Id resId = opt_boardResId.value();

            std::optional<DetectionResultMap::Id> promotedBoardId = model_->promoteToSnapshot(resId);
            if (!promotedBoardId.has_value())
            {
                UpdateLogs(Log("Board result " + wxString::Format("%" PRIu32, resId.getValue()) + " is no more in Model"));

                return;
            }

            std::optional<std::shared_ptr<DetectionResult>> boardRes = model_->getDetectionResult(resId);
            if (!boardRes.has_value())
            {
                UpdateLogs(Log("Board id " + wxString::Format("%" PRIu32, resId.getValue()) + " does not exists"));

                return;
            }

            UpdateBoard(promotedBoardId.value().get(), boardRes.value());

            viewState_->UnsetCurrentLiveFrame();
        }

        return;
    }

    const DetectionModel::SessionType sessionType = model_->sessionType();

    if (sessionType != DetectionModel::SessionType::UNDEFINED && sessionType != DetectionModel::SessionType::CAMERA)
    {
        if (!view_->AskYesNo("Starting a new session with a different type will clear all detection results.\nAre you sure?"))
        {
            return;
        }
    }

    view_->SetUiState(DetectionPage::UiState::BUSY);

    TaskEnqueueResult res = model_->startCameraSession(wxCameraID.ToStdString(), true);

    UpdateLogs(res.getLogs());

    if (res.getStatus() == TaskEnqueueResult::Status::REJECTED)
    {
        view_->SetUiState(DetectionPage::UiState::IDLE);
    }
}

void DetectionController::OnSnap(const wxCommandEvent& event)
{
    Snap();
}

//
void DetectionController::LoadImg()
{
    const DetectionModel::SessionType sessionType = model_->sessionType();

    if (sessionType != DetectionModel::SessionType::UNDEFINED && sessionType != DetectionModel::SessionType::FILE)
    {
        if (!view_->AskYesNo("Starting a new session with a different type will clear all detection results.\nAre you sure?"))
        {
            return;
        }
    }

    wxFileDialog openFileDialog(nullptr, "Load Image", "", "",
        "Image files (*.ima;*.tif;*.tiff;*.gif;*.bmp;*.jpg;*.jpeg;*.jp2;*.jxr;*.png;*.pcx;*.ras;*.xwd;*.pbm;*.pnm;*.pgm;*.ppm)|*.ima;*.tif;*.tiff;*.gif;*.bmp;*.jpg;*.jpeg;*.jp2;*.jxr;*.png;*.pcx;*.ras;*.xwd;*.pbm;*.pnm;*.pgm;*.ppm",
        wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE);
    if (openFileDialog.ShowModal() == wxID_CANCEL)
        return;

    wxArrayString filePaths;
    openFileDialog.GetPaths(filePaths);

    std::vector<std::string> tmp;
    tmp.reserve(filePaths.size());
    for (const auto& path : filePaths)
    {
        tmp.push_back(path.ToStdString());
    }

    if (tmp.empty()) return;

    view_->SetUiState(DetectionPage::UiState::BOARD_FROM_FILE);

    TaskEnqueueResult res = model_->startFileSession(tmp);

    if (res.getStatus() == TaskEnqueueResult::Status::REJECTED)
    {
        view_->SetUiState(DetectionPage::UiState::IDLE);
    }
}

void DetectionController::OnLoadImage(const wxCommandEvent& event)
{
    LoadImg();
}

//
void DetectionController::LiveCamera()
{
    const wxString wxCameraID = view_->GetSelectedCamera();

    if (wxCameraID.IsEmpty()) return;

    TaskEnqueueResult res;
    if (!model_->isSessionOn())
    {
        view_->SetUiState(DetectionPage::UiState::START_LIVE);

        const DetectionModel::SessionType sessionType = model_->sessionType();
        
        if (sessionType != DetectionModel::SessionType::UNDEFINED && sessionType != DetectionModel::SessionType::CAMERA)
        {
            if (!view_->AskYesNo("Starting a new session with a different type will clear all detection results.\nAre you sure?"))
            {
                view_->SetUiState(DetectionPage::UiState::IDLE);

                return;
            }
        }


        res = model_->startCameraSession(wxCameraID.ToStdString());

        if (res.getStatus() == TaskEnqueueResult::Status::REJECTED)
            view_->SetUiState(DetectionPage::UiState::IDLE);
    }
    else
    {
        view_->SetUiState(DetectionPage::UiState::STOP_LIVE);

        res = model_->stopSession();

        if (res.getStatus() == TaskEnqueueResult::Status::ALREADY_DONE)
            view_->SetUiState(DetectionPage::UiState::IDLE);
    }

    UpdateLogs(res.getLogs());
}

void DetectionController::OnLiveCamera(const wxCommandEvent& event)
{
    LiveCamera();
}

//
void DetectionController::ShowPerBoardDebugResult(const wxCommandEvent& event)
{
    std::optional<DetectionResult::Id> opt_boardResId = viewState_->GetCurrentRenderedBoard();

    // We do not want to display deb result if no board is rendered
    if (!opt_boardResId.has_value())
    {
        view_->ClearImageDisplay();
        return;
    }

    const DetectionResult::Id boardResId = opt_boardResId.value();
    const std::string pluginId = event.GetString().ToStdString();
    const bool isChecked = static_cast<bool>(event.GetInt());

    // Convert and display board deb result
    if (isChecked)
    {
        std::shared_ptr<DetectionResult> boardRes = model_->getDetectionResult(boardResId).value_or(
            model_->getLiveDetectionResult(boardResId).value_or(nullptr)
        );

        if (boardRes)
        {
            // Convert debug result
            std::optional<PluginDebugResultView> convRes = utils_->buildPluginDebugResult(pluginId, boardRes->evaluatedBoard());
            if (convRes.has_value())    // plugin deb converted, render it
            {
                view_->UpdateImageDisplay(convRes.value().debugImage());
            }
            else  // utilsLayer has no builder for said plugin...
            {
                // ...fallback to displaying its board image
                view_->UpdateImageDisplay(
                    utils_->convertBoardImageToWx(boardRes->evaluatedBoard()->object())
                );
            }
        }
    }
    else
    {
        std::shared_ptr<DetectionResult> boardRes = model_->getDetectionResult(boardResId).value_or(
            model_->getLiveDetectionResult(boardResId).value_or(nullptr)
        );

        if (boardRes)
        {
            view_->UpdateImageDisplay(
                utils_->convertBoardImageToWx(boardRes->evaluatedBoard()->object())
            );
        }
    }
}

void DetectionController::OnShowPerBoardDebugResult(const wxCommandEvent& event)
{
    ShowPerBoardDebugResult(event);
}

//
void DetectionController::ShowPerSequenceDebugResult(const wxCommandEvent& event)
{
    const std::string pluginId = event.GetString().ToStdString();
    const bool isChecked = static_cast<bool>(event.GetInt());

    std::optional<DetectionResult::Id> opt_boardResId = viewState_->GetCurrentRenderedBoard();

    // Convert and display board sequence deb result
    if (isChecked)
    {
        std::shared_ptr<EvaluatedBoardSequence> evalSeq = model_->getEvaluatedSequence();

        std::optional<PluginDebugResultView> convRes = utils_->buildPluginDebugResult(pluginId, evalSeq);
        if (convRes.has_value())    // plugin deb converted, render it
        {
            const PluginDebugResultView& deb = convRes.value();

            view_->UpdateImageDisplay(deb.debugImage());
        }
        else  // utilsLayer has no builder for said plugin...
        {
            // No board to display if no rendered board
            if (!opt_boardResId.has_value())
            {
                view_->ClearImageDisplay();
            }
            else // ...fallback to displaying rendered board image
            {
                DetectionResult::Id boardResId = opt_boardResId.value();

                std::shared_ptr<DetectionResult> boardRes = model_->getDetectionResult(boardResId).value_or(
                    model_->getLiveDetectionResult(boardResId).value_or(nullptr)
                );

                if (boardRes)
                {
                    view_->UpdateImageDisplay(
                        utils_->convertBoardImageToWx(boardRes->evaluatedBoard()->object())
                    );
                }
            }
        }
    }
    else
    {
        // No board to display if no rendered board
        if (!opt_boardResId.has_value())
        {
            view_->ClearImageDisplay();
        }
        else // ...fallback to displaying rendered board image
        {
            DetectionResult::Id boardResId = opt_boardResId.value();

            std::shared_ptr<DetectionResult> boardRes = model_->getDetectionResult(boardResId).value_or(
                model_->getLiveDetectionResult(boardResId).value_or(nullptr)
            );

            if (boardRes)
            {
                view_->UpdateImageDisplay(
                    utils_->convertBoardImageToWx(boardRes->evaluatedBoard()->object())
                );
            }
        }
    }
}

void DetectionController::OnShowPerSequenceDebugResult(const wxCommandEvent& event)
{
    ShowPerSequenceDebugResult(event);
}

//
void DetectionController::ShowPixelTooltip(const PixelEvent& event)
{
    if (!view_->IsShowDebPluginSelected())
        return;

    const wxPoint& pixel = event.GetPixel();
    const auto [pluginId, pluginLocation] = view_->GetSelectedDebPlugin().value();

    std::optional<wxString> tip;
    if (pluginLocation == EvaluationPanel::PluginLocation::PER_BOARD)
    {
        std::optional<DetectionResult::Id> opt_boardResId = viewState_->GetCurrentRenderedBoard();
        if (opt_boardResId.has_value())
        {
            DetectionResult::Id boardResId = opt_boardResId.value();

            std::shared_ptr<DetectionResult> boardRes = model_->getDetectionResult(boardResId).value_or(
                model_->getLiveDetectionResult(boardResId).value_or(nullptr)
            );

            if (boardRes)
            {
                tip = utils_->getPluginPixelTooltip(
                    pluginId.ToStdString(),
                    boardRes->evaluatedBoard(),
                    pixel
                );
            }
        }
    }
    else if (pluginLocation == EvaluationPanel::PluginLocation::PER_SEQUENCE)
    {
        tip = utils_->getPluginPixelTooltip(
            pluginId.ToStdString(),
            model_->getEvaluatedSequence(),
            pixel
        );
    }

    if (tip.has_value())
    {
        view_->SetPixelToolTip(tip.value());
    }
    else
    {
        view_->UnsetPixelToolTip();
    }
}

void DetectionController::OnShowPixelTooltip(const PixelEvent& event)
{
    ShowPixelTooltip(event);
}

void DetectionController::DrawBoard(const wxCommandEvent& event)
{
    const bool isChecked = static_cast<bool>(event.GetInt());

    if (utils_->shouldDrawBoard() == isChecked)
        return;

    utils_->drawBoard(isChecked);

    if (view_->IsShowDebPluginSelected())
        return;

    std::optional<DetectionResult::Id> opt_boardResId = viewState_->GetCurrentRenderedBoard();

    if (!opt_boardResId.has_value())
        return;

    DetectionResult::Id boardResId = opt_boardResId.value();

    std::shared_ptr<DetectionResult> boardRes = model_->getDetectionResult(boardResId).value_or(
        model_->getLiveDetectionResult(boardResId).value_or(nullptr)
    );

    if (boardRes)
    {
        view_->UpdateImageDisplay(
            utils_->convertBoardImageToWx(boardRes->evaluatedBoard()->object())
        );
    }
}

void DetectionController::DrawMarks(const wxCommandEvent& event)
{
    const bool isChecked = static_cast<bool>(event.GetInt());

    if (utils_->shouldDrawMarks() == isChecked)
        return;

    utils_->drawMarks(isChecked);

    if (view_->IsShowDebPluginSelected())
        return;

    std::optional<DetectionResult::Id> opt_boardResId = viewState_->GetCurrentRenderedBoard();

    if (!opt_boardResId.has_value())
        return;

    DetectionResult::Id boardResId = opt_boardResId.value();

    std::shared_ptr<DetectionResult> boardRes = model_->getDetectionResult(boardResId).value_or(
        model_->getLiveDetectionResult(boardResId).value_or(nullptr)
    );

    if (boardRes)
    {
        view_->UpdateImageDisplay(
            utils_->convertBoardImageToWx(boardRes->evaluatedBoard()->object())
        );
    }
}

void DetectionController::DrawWCS(const wxCommandEvent& event)
{
    const bool isChecked = static_cast<bool>(event.GetInt());

    if (utils_->shouldDrawWCS() == isChecked)
        return;

    utils_->drawWCS(isChecked);

    if (view_->IsShowDebPluginSelected())
        return;

    std::optional<DetectionResult::Id> opt_boardResId = viewState_->GetCurrentRenderedBoard();

    if (!opt_boardResId.has_value())
        return;

    DetectionResult::Id boardResId = opt_boardResId.value();

    std::shared_ptr<DetectionResult> boardRes = model_->getDetectionResult(boardResId).value_or(
        model_->getLiveDetectionResult(boardResId).value_or(nullptr)
    );

    if (boardRes)
    {
        view_->UpdateImageDisplay(
            utils_->convertBoardImageToWx(boardRes->evaluatedBoard()->object())
        );
    }
}

//
void DetectionController::ReloadParameters()
{
    const BoardPattern pattern = UtilityFunctions::enumFromInt<BoardPattern>(
        model_->getParameter("pattern_type", PatternParametersRegistry::CATEGORY())->getValue<int>()
    ).value();

    // Setup pattern
    {
        const std::vector<std::shared_ptr<ParameterInfo>> params = model_->getFilteredParams(PatternParametersRegistry::CATEGORY());

        view_->SetParameters(params, DetectionView::ParameterLocation::SetupPattern);

        for (const auto& p : params)
        {
            const std::string& pName = p->name();
            const std::string& pCat = p->category();

            view_->MarkParameterAsDirty(pName, pCat, model_->isParameterDirty(pName, pCat));
        }
    }

    // Setup geometry
    {
        std::vector<std::shared_ptr<ParameterInfo>> params;

        switch (pattern)
        {
            case BoardPattern::CHESSBOARD:
                 params = model_->getFilteredParams(ChessboardParametersRegistry::CATEGORY_GEOMETRY);

                break;

            case BoardPattern::SYMMETRIC_CIRCLES:
            case BoardPattern::ASYMMETRIC_CIRCLES:
                params = model_->getFilteredParams(CircleboardParametersRegistry::CATEGORY_GEOMETRY);

                break;

            case BoardPattern::CHARUCO:
                params = model_->getFilteredParams(ArucoParametersRegistry::CATEGORY_GEOMETRY());
                UtilityFunctions::moveInto(model_->getFilteredParams(CharucoParametersRegistry::CATEGORY_GEOMETRY), params);

                break;

            case BoardPattern::APRIL_TAG:
                params = model_->getFilteredParams(ArucoParametersRegistry::CATEGORY_GEOMETRY());
                UtilityFunctions::moveInto(model_->getFilteredParams(AprilTagParametersRegistry::CATEGORY_GEOMETRY), params);

                break;
        }

        view_->SetParameters(params, DetectionView::ParameterLocation::SetupGeometry);

        for (const auto& p : params)
        {
            const std::string& pName = p->name();
            const std::string& pCat = p->category();

            view_->MarkParameterAsDirty(pName, pCat, model_->isParameterDirty(pName, pCat));
        }
    }

    // Setup detection
    {
        std::vector<std::shared_ptr<ParameterInfo>> params;

        switch (pattern)
        {
            case BoardPattern::CHESSBOARD:
                params = model_->getFilteredParams(ChessboardParametersRegistry::CATEGORY_DETECTION);

                break;

            case BoardPattern::SYMMETRIC_CIRCLES:
            case BoardPattern::ASYMMETRIC_CIRCLES:
                params = model_->getFilteredParams(ChessboardParametersRegistry::CATEGORY_DETECTION);

                break;

            case BoardPattern::CHARUCO:
                params = model_->getFilteredParams(ArucoParametersRegistry::CATEGORY_DETECTION());
                UtilityFunctions::moveInto(model_->getFilteredParams(CharucoParametersRegistry::CATEGORY_DETECTION), params);

                break;

            case BoardPattern::APRIL_TAG:
                params = model_->getFilteredParams(ArucoParametersRegistry::CATEGORY_DETECTION());
                UtilityFunctions::moveInto(model_->getFilteredParams(AprilTagParametersRegistry::CATEGORY_DETECTION), params);

                break;
        }

        view_->SetParameters(std::move(params), DetectionView::ParameterLocation::SetupDetection);

        for (const auto& p : params)
        {
            const std::string& pName = p->name();
            const std::string& pCat = p->category();

            view_->MarkParameterAsDirty(pName, pCat, model_->isParameterDirty(pName, pCat));
        }
    }

    // Setup refine
    {
        std::vector<std::shared_ptr<ParameterInfo>> params;

        switch (pattern)
        {
            case BoardPattern::CHESSBOARD:
                params = model_->getFilteredParams(ChessboardParametersRegistry::CATEGORY_REFINE);

                break;

            case BoardPattern::SYMMETRIC_CIRCLES:
            case BoardPattern::ASYMMETRIC_CIRCLES:
                break;

            case BoardPattern::CHARUCO:
                params = model_->getFilteredParams(ArucoParametersRegistry::CATEGORY_REFINE());

                break;

            case BoardPattern::APRIL_TAG:
                params = model_->getFilteredParams(ArucoParametersRegistry::CATEGORY_REFINE());

                break;
        }

        view_->SetParameters(std::move(params), DetectionView::ParameterLocation::SetupRefine);

        for (const auto& p : params)
        {
            const std::string& pName = p->name();
            const std::string& pCat = p->category();

            view_->MarkParameterAsDirty(pName, pCat, model_->isParameterDirty(pName, pCat));
        }
    }

    // Pre-processing
    {
        const std::vector<std::shared_ptr<ParameterInfo>> params = model_->getFilteredParams(DetectionModelRegistry::CATEGORY_PRE_PROC);

        view_->SetParameters(std::move(params), DetectionView::ParameterLocation::PreProcessing);

        for (const auto& p : params)
        {
            const std::string& pName = p->name();
            const std::string& pCat = p->category();

            view_->MarkParameterAsDirty(pName, pCat, model_->isParameterDirty(pName, pCat));
        }
    }
}

//
void DetectionController::boardDetectionFromImage(const MessageTask& message)
{
    switch (message.status())
    {
    case MessageTask::TaskStatus::REJECTED:
        if (!uiTickProxy_->IsRunning() && canIdle())
        {
            view_->SetUiState(DetectionPage::UiState::IDLE);
        }

        break;

    case MessageTask::TaskStatus::PENDING:
        break;

    case MessageTask::TaskStatus::STARTED:
        view_->SetUiState(DetectionPage::UiState::BOARD_FROM_FILE);

        addDetectionTask();

        break;

    case MessageTask::TaskStatus::SUCCESS:
    case MessageTask::TaskStatus::FAIL:
        completeDetectionTask();

        if (!uiTickProxy_->IsRunning() && canIdle())
        {
            view_->SetUiState(DetectionPage::UiState::IDLE);
        }

        break;
    }

    UpdateLogsMessage(message);
}

void DetectionController::onBoardDetectionFromImage(const MessageTask& message)
{
    wxTheApp->CallAfter([this, message]() {
        boardDetectionFromImage(message);
        });
}

//
void DetectionController::boardDetectionFromSnap(const MessageTask& message)
{
    switch (message.status())
    {
    case MessageTask::TaskStatus::REJECTED:
        if (!uiTickProxy_->IsRunning() && canIdle())
        {
            view_->SetUiState(DetectionPage::UiState::IDLE);
        }

        break;

    case MessageTask::TaskStatus::PENDING:
        break;

    case MessageTask::TaskStatus::STARTED:
        view_->SetUiState(DetectionPage::UiState::BUSY);    // TODO -> ::BoardFromSnap

        addDetectionTask();

        break;

    case MessageTask::TaskStatus::SUCCESS:
    case MessageTask::TaskStatus::FAIL:
        completeDetectionTask();

        if (!uiTickProxy_->IsRunning() && canIdle())
        {
            view_->SetUiState(DetectionPage::UiState::IDLE);
        }

        break;
    }

    UpdateLogsMessage(message);
}

void DetectionController::onBoardDetectionFromSnap(const MessageTask& message)
{
    wxTheApp->CallAfter([this, message]() {
        boardDetectionFromSnap(message);
        });
}

//
void DetectionController::boardFromLive(const MessageTask& message)
{
    switch (message.status())
    {
    case MessageTask::TaskStatus::REJECTED:
        if (!uiTickProxy_->IsRunning() && canIdle())
        {
            view_->SetUiState(DetectionPage::UiState::IDLE);
        }

        break;

    case MessageTask::TaskStatus::PENDING:
        break;

    case MessageTask::TaskStatus::STARTED:
        view_->SetUiState(DetectionPage::UiState::LIVE);

        addDetectionTask();

        break;

    case MessageTask::TaskStatus::SUCCESS:
    case MessageTask::TaskStatus::FAIL:
        completeDetectionTask();

        if (!uiTickProxy_->IsRunning() && canIdle())
        {
            view_->SetUiState(DetectionPage::UiState::IDLE);
        }

        viewState_->UnsetCurrentLiveFrame();

        break;
    }

    UpdateLogsMessage(message);
}

void DetectionController::onBoardFromLive(const MessageTask& message)
{
    wxTheApp->CallAfter([this, message]() {
        boardFromLive(message);
        });
}

//
void DetectionController::boardReDetection(const MessageTask& message)
{
    switch (message.status())
    {
    case MessageTask::TaskStatus::REJECTED:
    case MessageTask::TaskStatus::PENDING:
        break;

    case MessageTask::TaskStatus::STARTED:
    if (view_->GetUiState() == DetectionPage::UiState::IDLE)
        view_->SetUiState(DetectionPage::UiState::BOARD_UPDATE);

        addDetectionTask();

        break;

    case MessageTask::TaskStatus::SUCCESS:
    case MessageTask::TaskStatus::FAIL:
        completeDetectionTask();

        if (!uiTickProxy_->IsRunning() && canIdle() && view_->GetUiState() == DetectionPage::UiState::BOARD_UPDATE)
        {
            view_->SetUiState(DetectionPage::UiState::IDLE);
        }

        break;
    }

    UpdateLogsMessage(message);
}

void DetectionController::onBoardReDetection(const MessageTask& message)
{
    wxTheApp->CallAfter([this, message]() {
        boardReDetection(message);
        });
}

//
void DetectionController::boardReEvaluation(const MessageTask& message)
{
    switch (message.status())
    {
    case MessageTask::TaskStatus::REJECTED:
    case MessageTask::TaskStatus::PENDING:
        break;

    case MessageTask::TaskStatus::STARTED:
    if (view_->GetUiState() == DetectionPage::UiState::IDLE)
        view_->SetUiState(DetectionPage::UiState::BOARD_REEVALUATE);

        addDetectionTask();

        break;

    case MessageTask::TaskStatus::SUCCESS:
    case MessageTask::TaskStatus::FAIL:
        completeDetectionTask();

        if (!uiTickProxy_->IsRunning() && canIdle() && view_->GetUiState() == DetectionPage::UiState::BOARD_REEVALUATE)
        {
            view_->SetUiState(DetectionPage::UiState::IDLE);
        }

        break;
    }

    UpdateLogsMessage(message);
}

void DetectionController::onBoardReEvaluation(const MessageTask& message)
{
    wxTheApp->CallAfter([this, message]() {
        boardReEvaluation(message);
        });
}

//
void DetectionController::boardStored(const MessageP<DetectionResultMap::Entry>& message)
{
    utils_->cook(message.getPayload());

    UpdateLogsMessage(message);
}

void DetectionController::onBoardStored(const MessageP<DetectionResultMap::Entry>& message)
{
    wxTheApp->CallAfter([this, message]() {
        boardStored(message);
        });
}

//
void DetectionController::boardUpdate(const MessageP<DetectionResultMap::Entry>& message)
{
    utils_->cook(message.getPayload());

    UpdateLogsMessage(message);
}

void DetectionController::onBoardUpdate(const MessageP<DetectionResultMap::Entry>& message)
{
    wxTheApp->CallAfter([this, message]() {
        boardUpdate(message);
        });
}

//
void DetectionController::boardSequenceReEvaluated(const MessageP<std::shared_ptr<EvaluatedBoardSequence>>& message)
{
    if (view_->GetUiState() == DetectionPage::UiState::IDLE)
        view_->SetUiState(DetectionPage::UiState::SEQUENCE_REEVALUATE);

    requestStartTimer();

    utils_->cook(message.getPayload());

    UpdateLogsMessage(message);
}

void DetectionController::onBoardSequenceReEvaluated(const MessageP<std::shared_ptr<EvaluatedBoardSequence>>& message)
{
    wxTheApp->CallAfter([this, message]() {
        boardSequenceReEvaluated(message);
        });
}

//
void DetectionController::onPatternTypeChanged(const Message& message)
{
    wxTheApp->CallAfter([this, message]() {
        ReloadParameters();

        UpdateLogsMessage(message);
        });
}

//
void DetectionController::onSessionCleared(const Message& message)
{
    wxTheApp->CallAfter([this, message]() {
        ClearSessionView();

        UpdateLogsMessage(message);
        });
}
