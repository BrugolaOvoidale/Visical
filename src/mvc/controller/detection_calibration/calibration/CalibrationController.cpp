#include "CalibrationController.hpp"
#include <cinttypes>
#include <wx/app.h>
#include <CvToWxImageConverter.hpp>
#include <calibration/Board.hpp>
#include <mvc/model/detection_calibration/SharedCameraIntrinsics.hpp>
#include <mvc/model/detection_calibration/calibration/CalibrationModel.hpp>
#include <mvc/view/detection_calibration/calibration/CalibrationPage.hpp>
#include <mvc/view/detection_calibration/calibration/CalibrationPageEvents.hpp>
#include <mvc/view/detection_calibration/common/board/BoardWidgetDTO.hpp>
#include <mvc/view/detection_calibration/common/board/BoardEvent.hpp>
#include "CalibrationSettings.hpp"
#include "CalibrationUtility.hpp"
#include "../../BaseControllerLogger.hpp"


static const wxString SNAP_TAG  { "SNAP" };
static const wxString LIVE_TAG  { "LIVE" };

static const std::string METADATA_IMAGE_TIMESTAMP{ "Image timestamp" };

/////////////////////////////////////////////////////

CalibrationController::CalibrationController(
    CalibrationPage* view,
    const std::shared_ptr<SharedCameraIntrinsics>& camIntrinsics,
    const std::shared_ptr<BaseControllerLogger>& logger)
    : CalibrationStageController(
        CalibrationModel::create(camIntrinsics, logger),
        view,
        std::make_shared<CalibrationSettings>(),
        std::make_shared<CalibrationUtility>(),
        logger)
{
    model_ = std::static_pointer_cast<CalibrationModel>(model());

    settings_ = std::static_pointer_cast<CalibrationSettings>(settings());

    view_ = view;

    utils_ = std::static_pointer_cast<CalibrationUtility>(utils());

    CalibrationController::init();

    CalibrationStageController::init();
}

/////////////////////////////////////////////////////
 
bool CalibrationController::setWorkingDataset(const std::vector<std::shared_ptr<Board>>& boards)
{
    if (model_->countBoards() != 0)
    {
        const bool answer = view_->AskYesNo("Setting a new working dataset will remove all the currently loaded boards. Do you want to proceed?");

        if (!answer)
			return false;
    }

    model_->setWorkingDataset(boards);

    view_->RemoveAllBoards();

    view_->ClearImageDisplay();

    view_->ClearAllPlugins(EvaluationPanel::PluginLocation::PER_BOARD);
    view_->ClearAllPlugins(EvaluationPanel::PluginLocation::PER_SEQUENCE);

    view_->SetCalibrationError(-1.0);

    view_->SetParameters(
        {},
        CalibrationPage::ParameterLocation::CAMERA_MATRIX
    );

    view_->SetParameters(
        {},
        CalibrationPage::ParameterLocation::DISTORTION_MODEL
    );

    UpdateBoards(model_->getWorkingDataset());

    view_->GoToPage(CalibrationPage::Page::CALIBRATION);

    view_->UnselectBoard();

    if (view_->IsAutoCalibrateOnNewDatasetChecked())
    {
        StartCalibration();
    }

    return true;
}

/////////////////////////////////////////////////////

void CalibrationController::init()
{
    // Model
    model_->subscribe(MSG_CALIBRATE_CAMERA, &CalibrationController::onCameraCalibration, this);
    model_->subscribe(MSG_CALIBRATION_PARAMETERS_CHANGED, &CalibrationController::onCalibrationParametersChanged, this);
    model_->subscribe(MSG_CALIB_BOARD_REEVALUATION, &CalibrationController::onCalibrationBoardReevaluated, this);
    model_->subscribe(MSG_CAM_MODEL_REEVALUATION, &CalibrationController::onCamModelReevaluated, this);


    // View
    view_->Bind(GUI_START_CALIBRATION, &CalibrationController::OnStartCalibration, this);
    view_->Bind(GUI_SAVE_CALIBRATION, &CalibrationController::OnSaveCalibrationResult, this);
    view_->Bind(GUI_UNDISTORT, &CalibrationController::OnUndistort, this);

    doLoadSettingsImpl();

    view_->SetParameters(model_->getCalibrationParametersInfo(), CalibrationPage::ParameterLocation::SETUP);
}

void CalibrationController::UpdateBoard(const BoardMap::Entry& board)
{
    BoardWidgetDTO boardDTO(
        board.first.get(),
        wxEmptyString,
        BoardWidgetDTO::BoardStatus::NONE,
        BoardWidgetDTO::EvaluationStatus::NOT_EVALUATED
    );

    const bool updateSuccess = view_->UpdateBoard(boardDTO);

    if (!updateSuccess)
    {
        view_->AppendBoard(boardDTO);
    }
}

void CalibrationController::UpdateBoards(const std::map<BoardMap::Id, std::shared_ptr<Board>>& boards)
{
    view_->Freeze();

    for (const auto& b : boards) UpdateBoard(b);

    view_->Thaw();
}

void CalibrationController::UpdateBoard(const EvaluatedCalibratedBoardEntry& evalCalibBoard)
{
    BoardWidgetDTO::EvaluationStatus evalStatus;
    switch (evalCalibBoard.evaluatedCalibratedBoard().status())
    {
    case EvaluatedCalibratedBoard::Status::NOT_EVALUATED:
        evalStatus = BoardWidgetDTO::EvaluationStatus::NOT_EVALUATED;

        break;

    case EvaluatedCalibratedBoard::Status::GOOD:
        evalStatus = BoardWidgetDTO::EvaluationStatus::GOOD;

        break;

    case EvaluatedCalibratedBoard::Status::HAS_ISSUES:
        evalStatus = BoardWidgetDTO::EvaluationStatus::HAS_ISSUES;

        break;
    }

    BoardWidgetDTO boardDTO(
        evalCalibBoard.mapId().get(),
        wxString::Format("RMS error: %.4f", evalCalibBoard.evaluatedCalibratedBoard().object().RMSerror()),
        BoardWidgetDTO::BoardStatus::NONE,
        evalStatus
    );

    const bool updateSuccess = view_->UpdateBoard(boardDTO);

    if (!updateSuccess)
    {
        view_->AppendBoard(boardDTO);
    }
}

void CalibrationController::UpdateBoards(const std::vector<EvaluatedCalibratedBoardEntry>& evalCalibBoards)
{
    view_->Freeze();

    for (const auto& e : evalCalibBoards) UpdateBoard(e);

    view_->Thaw();
}

//
void CalibrationController::doLoadSettingsImpl()
{    
    TaskResultP<CalibrationSettings::Loaded> loadRes = settings_->loadSettings();

    UpdateLogs(loadRes.getLogs());

    if (loadRes.isSuccess())
    {
        const CalibrationSettings::Loaded& payload = loadRes.getPayload();

        view_->CheckAutoCalibrateOnNewDataset(payload.autoCalibOnNewDataset);

        view_->DrawBoard(payload.drawBoard);
        view_->DrawMarks(payload.drawMarks);
        view_->DrawWCS(payload.drawWCS);

        utils_->drawBoard(payload.drawBoard);
        utils_->drawMarks(payload.drawMarks);
        utils_->drawWCS(payload.drawWCS);
    }
}

//
void CalibrationController::doSaveSettingsImpl()
{
    TaskResult res = settings_->saveSettings(
        view_->IsAutoCalibrateOnNewDatasetChecked(),
        view_->IsDrawBoardEnabled(),
        view_->IsDrawMarksEnabled(),
        view_->IsDrawWCSEnabled()
    );

    UpdateLogs(res.takeLogs());
}


//
void CalibrationController::SelectBoard(const BoardEvent& event)
{
    if (!view_->IsBoardSelected())
    {
        view_->ClearImageDisplay();
        view_->ClearAllPlugins(EvaluationPanel::PluginLocation::PER_BOARD);

        return;
    }

    const std::uint32_t boardMapId = event.GetBoardId();
    const BoardMap::Id mapId(boardMapId);

    std::optional<std::shared_ptr<Board>> opt_board = model_->getBoard(mapId);
    if (!opt_board.has_value())
    {
        UpdateLogs(Log("Board id " + wxString::Format("%" PRIu32, boardMapId) + " does not exists"));

        return;
    }

    std::shared_ptr<Board> board = opt_board.value();

    wxImage img;
    if (view_->IsUndistortChecked())
    {
        TaskResultP<CvImage> undistortedImage = model_->undistort(board->image());

        if (undistortedImage.isSuccess())
        {
            img = CvToWxImageConverter::toWxImage(undistortedImage.getPayload());
        }
        else
        {
            img = utils_->convertBoardImageToWx(board);
        }

        UpdateLogs(undistortedImage.getLogs());
    }
    else
    {
        img = utils_->convertBoardImageToWx(board);
    }

    view_->UpdateImageDisplay(img);

    std::optional<CalibrationResult> opt_calibRes = model_->getLastCalibrationResult();
    if (!opt_calibRes.has_value())
        return;

    const CalibrationResult& calibRes = opt_calibRes.value();
    for (const auto& e : calibRes.evaluatedBoards())
    {
        if (e.mapId() == boardMapId)
        {
            UpdatePluginResults(e.evaluatedCalibratedBoard().assessments());

            break;
        }
    }
}

//
void CalibrationController::RemoveBoard(const BoardEvent& event)
{
    const std::uint32_t boardId = event.GetBoardId();
    const BoardMap::Id mapId(boardId);

    view_->RemoveBoard(boardId);

    if (view_->IsBoardSelected() && view_->GetSelectedBoard().value() == boardId)
    {
        view_->ClearImageDisplay();
        view_->ClearAllPlugins(EvaluationPanel::PluginLocation::PER_BOARD);
        view_->UnselectBoard();
    }

    std::optional<BoardMap::Id> newIdx = model_->removeBoard(mapId);

    if (!newIdx.has_value())
    {
        UpdateLogs(Log("Board id " + wxString::Format("%" PRIu32, boardId) + std::string(" doesn't exists in Model")));
        return;
    }

    BoardMap::Id newIdx_value = newIdx.value();
    if (newIdx_value > 0)
    {
        std::optional<std::shared_ptr<Board>> boardRes = model_->getBoard(newIdx_value);
        if (!boardRes.has_value())
        {
            UpdateLogs(Log("Board id " + wxString::Format("%" PRIu32, newIdx_value.get()) + " does not exists"));

            return;
        }

        const std::shared_ptr<Board>& board = boardRes.value();

        //UpdatePluginResults(evalBoard->assessments());

        view_->SelectBoard(newIdx_value.get());
    }
    else
    {
        view_->ClearAllPlugins(EvaluationPanel::PluginLocation::PER_BOARD);
    }
}

//
void CalibrationController::RemoveAllBoards()
{
    model_->clearDataset();

    view_->RemoveAllBoards();
    view_->ClearImageDisplay();
    view_->ClearAllPlugins(EvaluationPanel::PluginLocation::PER_BOARD);
    view_->ClearAllPlugins(EvaluationPanel::PluginLocation::PER_SEQUENCE);
    view_->UnselectBoard();
}

//
void CalibrationController::ReloadParameters()
{
    // Setup
    {
        const std::vector<std::shared_ptr<ParameterInfo>> params = model_->getCalibrationParametersInfo();

        view_->SetParameters(params, CalibrationPage::ParameterLocation::SETUP);
        
        for (const auto& p : params)
        {
            const std::string& pName = p->name();
            const std::string& pCat = p->category();

            view_->MarkParameterAsDirty(pName, pCat, model_->isParameterDirty(pName, pCat));
        }
    }
}

void CalibrationController::DrawBoard(const wxCommandEvent& event)
{
    const bool isChecked = static_cast<bool>(event.GetInt());

    if (utils_->shouldDrawBoard() == isChecked)
        return;

    utils_->drawBoard(isChecked);

    if (view_->IsUndistortChecked())
        return;

    if (view_->IsShowDebPluginSelected())
        return;

    std::optional<std::uint32_t> boardId = view_->GetSelectedBoard();
    if (!boardId.has_value())
        return;

    std::optional<std::shared_ptr<Board>> board = model_->getBoard(BoardMap::Id(boardId.value()));
    if (!board.has_value())
        return;

    view_->UpdateImageDisplay(
        utils_->convertBoardImageToWx(board.value())
    );
}

void CalibrationController::DrawMarks(const wxCommandEvent& event)
{
    const bool isChecked = static_cast<bool>(event.GetInt());

    if (utils_->shouldDrawMarks() == isChecked)
        return;

    utils_->drawMarks(isChecked);

    if (view_->IsUndistortChecked())
        return;

    if (view_->IsShowDebPluginSelected())
        return;

    std::optional<std::uint32_t> boardId = view_->GetSelectedBoard();
    if (!boardId.has_value())
        return;

    std::optional<std::shared_ptr<Board>> board = model_->getBoard(BoardMap::Id(boardId.value()));
    if (!board.has_value())
        return;

    view_->UpdateImageDisplay(
        utils_->convertBoardImageToWx(board.value())
    );
}

void CalibrationController::DrawWCS(const wxCommandEvent& event)
{
    const bool isChecked = static_cast<bool>(event.GetInt());

    if (utils_->shouldDrawWCS() == isChecked)
        return;

    utils_->drawWCS(isChecked);

    if (view_->IsUndistortChecked())
        return;

    if (view_->IsShowDebPluginSelected())
        return;

    std::optional<std::uint32_t> boardId = view_->GetSelectedBoard();
    if (!boardId.has_value())
        return;

    std::optional<std::shared_ptr<Board>> board = model_->getBoard(BoardMap::Id(boardId.value()));
    if (!board.has_value())
        return;

    view_->UpdateImageDisplay(
        utils_->convertBoardImageToWx(board.value())
    );
}

//
void CalibrationController::StartCalibration()
{
    view_->SetUiState(CalibrationPage::UiState::CALIBRATION);
    
    TaskEnqueueResult res = model_->calibrateCamera();

    if (res.getStatus() != TaskEnqueueResult::Status::NO_ERRORS)
    {
        view_->SetUiState(CalibrationPage::UiState::IDLE);
    }

    UpdateLogs(res.getLogs());
}

void CalibrationController::OnStartCalibration(const wxCommandEvent& event)
{
    StartCalibration();
}

//
void CalibrationController::SaveCalibrationResult()
{
    std::optional<CalibrationResult> calibRes = model_->getLastCalibrationResult();
    if (!calibRes.has_value())
        return;

    wxString message = "Save .json File";
    wxString wildCard = "JSON files (*.json;*.txt)|*.json;*.txt";

    std::optional<wxString> wxFilePath = view_->SaveFileDialog(message, wildCard);
    if (!wxFilePath.has_value())
        return; // user canceled save dialog


    const std::string filePath = wxFilePath.value().ToStdString();

    TaskResult saveRes = CalibrationUtility::saveCalibrationResult(filePath, calibRes.value());

    if (!saveRes.isSuccess())
    {
        view_->ShowError("Failed to save calibration result");
    }
    else
    {
        view_->ShowSuccess("Successfully saved calibration result in " + filePath);
    }

    UpdateLogs(saveRes.getLogs());
}

void CalibrationController::OnSaveCalibrationResult(const wxCommandEvent& event)
{
    SaveCalibrationResult();
}

//
void CalibrationController::Undistort(const wxCommandEvent& event)
{
    if (!view_->IsBoardSelected())
        return;

    BoardMap::Id mapId(view_->GetSelectedBoard().value());

    std::optional<std::shared_ptr<Board>> opt_board = model_->getBoard(mapId);
    if (!opt_board.has_value())
    {
        UpdateLogs(Log("Board id " + wxString::Format("%" PRIu32, mapId.get()) + " does not exists"));

        return;
    }

    std::shared_ptr<Board> board = opt_board.value();

    const bool isChecked = static_cast<bool>(event.GetInt());

    if (isChecked)
    {
        TaskResultP<CvImage> undistortedImage = model_->undistort(board->image());

        if (!undistortedImage.isSuccess())
        {
            UpdateLogs(undistortedImage.getLogs());
            return;
        }

        view_->UpdateImageDisplay(
            CvToWxImageConverter::toWxImage(undistortedImage.getPayload())
        );
    }
    else
    {
        std::optional<std::shared_ptr<Board>> board = model_->getBoard(mapId);

        if (board.has_value())
        {
            view_->UpdateImageDisplay(
                utils_->convertBoardImageToWx(board.value())
            );
        }
        else
        {
            UpdateLogs(Log("Board id " + wxString::Format("%" PRIu32, mapId.get()) + " does not exists"));

            view_->ClearImageDisplay();
        }
    }

}

void CalibrationController::OnUndistort(const wxCommandEvent& event)
{
    Undistort(event);
}

//
void CalibrationController::cameraCalibration(const MessageTaskP<CalibrationResult>& msg)
{
    switch (msg.status())
    {
        case MessageTask::TaskStatus::PENDING:
        case MessageTask::TaskStatus::REJECTED:
        case MessageTask::TaskStatus::STARTED:
            break;

        case MessageTask::TaskStatus::SUCCESS:
        {
            CalibrationResult calibRes = msg.getPayload();
                
            view_->SetCalibrationError(calibRes.evaluatedCameraModel().object().reprojectionError());

            view_->SetParameters(
                calibRes.cameraMatrix(),
                CalibrationPage::ParameterLocation::CAMERA_MATRIX
            );

            view_->SetParameters(
                calibRes.distortionModel(),
                CalibrationPage::ParameterLocation::DISTORTION_MODEL
            );

            const std::vector<EvaluatedCalibratedBoardEntry>& evalBoards = calibRes.evaluatedBoards();


            UpdateBoards(evalBoards);

            for (const auto& e : evalBoards)
            {
                UpdatePluginResults(e.evaluatedCalibratedBoard().assessments());
            }

            UpdatePluginResults(calibRes.evaluatedCameraModel().assessments());
        }
        case MessageTask::TaskStatus::FAIL:
            view_->SetUiState(CalibrationPage::UiState::IDLE);

            break;
    }

    UpdateLogsMessage(msg);
}

void CalibrationController::onCameraCalibration(const MessageTaskP<CalibrationResult>& msg)
{
    wxTheApp->CallAfter([this, msg]() {
        cameraCalibration(msg);
        });
}

//
void CalibrationController::calibrationParametersChanged(const Message& msg)
{
    view_->SetParameters(
        model_->getCalibrationParametersInfo(),
        CalibrationPage::ParameterLocation::SETUP
    );

    UpdateLogsMessage(msg);
}

void CalibrationController::onCalibrationParametersChanged(const Message& msg)
{
    wxTheApp->CallAfter([this, msg]() {
        calibrationParametersChanged(msg);
        });
}

void CalibrationController::calibrationBoardReevaluated(const MessageP<std::vector<EvaluatedCalibratedBoardEntry>>& msg)
{
    const std::vector<EvaluatedCalibratedBoardEntry>& evalBoards = msg.getPayload();

    for (const auto& e : evalBoards)
    {
        UpdatePluginResults(e.evaluatedCalibratedBoard().assessments());
    }
}

void CalibrationController::onCalibrationBoardReevaluated(const MessageP<std::vector<EvaluatedCalibratedBoardEntry>>& msg)
{
    wxTheApp->CallAfter([this, msg]() {
        calibrationBoardReevaluated(msg);
        });
}

void CalibrationController::camModelReevaluated(const MessageP<EvaluatedCameraModel>& msg)
{
    UpdatePluginResults(msg.getPayload().assessments());
}

void CalibrationController::onCamModelReevaluated(const MessageP<EvaluatedCameraModel>& msg)
{
    wxTheApp->CallAfter([this, msg]() {
        camModelReevaluated(msg);
        });
}
