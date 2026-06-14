#include "CalibrationStageController.hpp"
#include <evaluator/PluginInfo.hpp>
#include <evaluator/PluginContextInfo.hpp>
#include <mvc/view/detection_calibration/common/CalibrationStageView.hpp>
#include <mvc/view/detection_calibration/common/CalibrationStageViewEvents.hpp>
#include <mvc/view/detection_calibration/common/evaluation/EvaluationPluginWidgetEvents.hpp>
#include <mvc/view/detection_calibration/common/evaluation/EvaluationPanelEvents.hpp>
#include <mvc/view/detection_calibration/common/board/BoardWidgetEvents.hpp>
#include <mvc/model/detection_calibration/common/CalibrationStageModel.hpp>
#include <gui_elements/parameter/ParameterWidgetEvents.hpp>
#include "CalibrationStageControllerSettings.hpp"
#include "CalibrationStageUtility.hpp"


CalibrationStageController::CalibrationStageController(
    std::shared_ptr<CalibrationStageModel> model,
    CalibrationStageView* view,
    std::shared_ptr<CalibrationStageControllerSettings> settings,
    std::shared_ptr<CalibrationStageUtility> utils,
    const std::shared_ptr<BaseControllerLogger>& logger)
    : BaseController(logger)
{
	if (!model)
        throw std::invalid_argument("model is nullptr");

    if (!view)
		throw std::invalid_argument("view is nullptr");

    if (!settings)
		throw std::invalid_argument("settings is nullptr");

    if (!utils)
		throw std::invalid_argument("utils is nullptr");

	model_ = model;
	view_ = view;
	settings_ = settings;
	utils_ = utils;
}

/////////////////////////////////////////////////////

void CalibrationStageController::shutdown()
{
    model_->BaseModel::shutdown();
}

/////////////////////////////////////////////////////

void CalibrationStageController::init()
{
    // Event bindings
    view_->Bind(GUI_DRAW_BOARD, &CalibrationStageController::OnDrawBoard, this);
    view_->Bind(GUI_DRAW_MARKS, &CalibrationStageController::OnDrawMarks, this);
    view_->Bind(GUI_DRAW_WCS, &CalibrationStageController::OnDrawWCS, this);

    view_->Bind(GUI_SET_PARAM_VALUE, &CalibrationStageController::OnEditParam, this);
    view_->Bind(GUI_RESET_PARAM, &CalibrationStageController::OnResetParam, this);
    view_->Bind(GUI_SET_PLUGIN_PARAM_VALUE, &CalibrationStageController::OnEditPluginParam, this);
    view_->Bind(GUI_ENABLE_DISABLE_EVAL_PLUGIN, &CalibrationStageController::OnEnablePlugin, this);
    view_->Bind(GUI_SET_THRESHOLD_EVAL_PLUGIN, &CalibrationStageController::OnChangePluginThreshold, this);
    view_->Bind(GUI_RESET_PLUGIN_PARAM, &CalibrationStageController::OnResetPluginParam, this);

    view_->Bind(GUI_BOARD_CLICK, &CalibrationStageController::OnSelectBoard, this);

    view_->Bind(GUI_LOAD_SETTINGS, &CalibrationStageController::OnLoadSettings, this);
    view_->Bind(GUI_SAVE_SETTINGS, &CalibrationStageController::OnSaveSettings, this);
    view_->Bind(GUI_LOAD_MODEL_PARAMS, &CalibrationStageController::OnLoadModelConfigFile, this);
    view_->Bind(GUI_SAVE_MODEL_PARAMS, &CalibrationStageController::OnSaveModelConfigFile, this);

    view_->Bind(GUI_BOARD_ADD_PLUGIN, &CalibrationStageController::OnBoardAddPlugin, this);
    view_->Bind(GUI_BOARD_REMOVE_PLUGIN, &CalibrationStageController::OnBoardRemovePlugin, this);
    view_->Bind(GUI_BOARD_LOAD_PLUGINS, &CalibrationStageController::OnLoadBoardPlugins, this);
    view_->Bind(GUI_BOARD_SAVE_PLUGINS, &CalibrationStageController::OnSaveBoardPlugins, this);
    view_->Bind(GUI_BOARD_GLOBAL_THR, &CalibrationStageController::OnChangeGlobalBoardThreshold, this);

    view_->Bind(GUI_SEQUENCE_ADD_PLUGIN, &CalibrationStageController::OnBoardSequenceAddPlugin, this);
    view_->Bind(GUI_SEQUENCE_REMOVE_PLUGIN, &CalibrationStageController::OnBoardSequenceRemovePlugin, this);
    view_->Bind(GUI_SEQUENCE_LOAD_PLUGINS, &CalibrationStageController::OnLoadBoardSequencePlugins, this);
    view_->Bind(GUI_SEQUENCE_SAVE_PLUGINS, &CalibrationStageController::OnSaveBoardSequencePlugins, this);
    view_->Bind(GUI_SEQUENCE_GLOBAL_THR, &CalibrationStageController::OnChangeGlobalBoardSeqThreshold, this);

    
    doLoadModelConfigFile(settings_->getModelParamsPath());

    doLoadBoardPlugins(settings_->getBoardPluginsPath());

    doLoadBoardSequencePlugins(settings_->getSequencePluginsPath());
    
    ReloadParameters();
}

std::shared_ptr<CalibrationStageModel> CalibrationStageController::model() const
{
	return model_;
}

CalibrationStageView* CalibrationStageController::view() const
{
    return view_;
}

std::shared_ptr<CalibrationStageControllerSettings> CalibrationStageController::settings() const
{
    return settings_;
}

std::shared_ptr<CalibrationStageUtility> CalibrationStageController::utils() const
{
    return utils_;
}

void CalibrationStageController::AppendPlugins(
    const std::vector<std::shared_ptr<PluginContextInfo>>& plugins,
    EvaluationPanel::PluginLocation where)
{
    view_->EnableAllPluginsPreview(where);

    for (const auto& ctx : plugins)
    {
        view_->AppendPlugin(ctx, where);
        view_->DisablePluginPreview(ctx->getPluginInfo()->id().data(), where);
    }
}

void CalibrationStageController::UpdatePluginResults(const std::vector<std::shared_ptr<PluginResult>>& pluginResults)
{
    for (const auto& r : pluginResults)
    {
        view_->UpdatePluginResult(r);
    }
}

void CalibrationStageController::doLoadSettings()
{
    doLoadSettingsImpl();

    doLoadModelConfigFile(settings_->getModelParamsPath());

    doLoadBoardPlugins(settings_->getBoardPluginsPath());

    doLoadBoardSequencePlugins(settings_->getSequencePluginsPath());
    
    ReloadParameters();
}

void CalibrationStageController::LoadSettings()
{
    wxString message = "Open .json File";
    wxString wildCard = "JSON files (*.json;*.txt)|*.json;*.txt";

    std::optional<wxString> wxFilePath = view_->OpenFileDialog(message, wildCard);
    if (!wxFilePath.has_value())
        return; // user canceled save dialog


    settings_->setSettingsPath(wxFilePath.value().ToStdString());

    doLoadSettings();
}

void CalibrationStageController::OnLoadSettings(const wxCommandEvent& event)
{
    LoadSettings();
}

void CalibrationStageController::doSaveSettings()
{
    doSaveSettingsImpl();

    doSaveModelConfigFile(settings_->getModelParamsPath());

    doSaveBoardPlugins(settings_->getBoardPluginsPath());

    doSaveBoardSequencePlugins(settings_->getSequencePluginsPath());
}

void CalibrationStageController::SaveSettings()
{
    wxString message = "Save .json File";
    wxString wildCard = "JSON files (*.json;*.txt)|*.json;*.txt";

    std::optional<wxString> wxFilePath = view_->SaveFileDialog(message, wildCard);
    if (!wxFilePath.has_value())
        return; // user canceled save dialog

    settings_->setSettingsPath(wxFilePath.value().ToStdString());

    doSaveSettings();
}

void CalibrationStageController::OnSaveSettings(const wxCommandEvent& event)
{
    SaveSettings();
}

//
void CalibrationStageController::OnDrawBoard(wxCommandEvent& event)
{
    DrawBoard(event);
}

//
void CalibrationStageController::OnDrawMarks(wxCommandEvent& event)
{
    DrawMarks(event);
}

//
void CalibrationStageController::OnDrawWCS(wxCommandEvent& event)
{
    DrawWCS(event);
}

//
void CalibrationStageController::doLoadModelConfigFile(const std::string& filePath)
{
    TaskResultP<std::vector<ParameterOwner::DTO>> res = utils_->loadModelParams(filePath);

    UpdateLogs(res.takeLogs());

    if (res.isSuccess())
    {
        settings_->setModelParamsPath(filePath);

        TaskResult setRes = model_->setParameters(res.getPayload());
        
        model_->saveParameters();
        
        ReloadParameters();

        UpdateLogs(setRes.takeLogs());
    }
}

void CalibrationStageController::LoadModelConfigFile()
{
    wxString message = "Open .json File";
    wxString wildCard = "JSON files (*.json;*.txt)|*.json;*.txt";

    std::optional<wxString> wxFilePath = view_->OpenFileDialog(message, wildCard);
    if (!wxFilePath.has_value())
        return; // user canceled save dialog

    doLoadModelConfigFile(wxFilePath.value().ToStdString());
}

void CalibrationStageController::OnLoadModelConfigFile(const wxCommandEvent& event)
{
    LoadModelConfigFile();
}

//
void CalibrationStageController::doSaveModelConfigFile(const std::string& filePath)
{
    TaskResult res = utils_->saveModelParams(filePath, model_->getFilteredParams());

    UpdateLogs(res.takeLogs());

    if (res.isSuccess())
    {
        model_->saveParameters();

        settings_->setModelParamsPath(filePath);

        ReloadParameters();
    }
}

void CalibrationStageController::SaveModelConfigFile()
{
    wxString message = "Save .json File";
    wxString wildCard = "JSON files (*.json;*.txt)|*.json;*.txt";

    std::optional<wxString> wxFilePath = view_->SaveFileDialog(message, wildCard);
    if (!wxFilePath.has_value())
        return; // user canceled save dialog

    const std::string filePath = wxFilePath.value().ToStdString();

    doSaveModelConfigFile(filePath);
}

void CalibrationStageController::OnSaveModelConfigFile(const wxCommandEvent& event)
{
    SaveModelConfigFile();
}

//
void CalibrationStageController::BoardAddPlugin(const wxCommandEvent& event)
{
    const wxString& wxPluginId = event.GetString();
    const std::string pluginId = wxPluginId.ToStdString();

    TaskResult res = model_->registerBoardPlugin(pluginId);

    if (res.isSuccess())
    {
        std::shared_ptr<PluginContextInfo> ctx = model_->getPluginContext(pluginId);
        if (!ctx)
            return;

        view_->AppendPlugin(ctx, EvaluationPanel::PluginLocation::PER_BOARD);
        view_->DisablePluginPreview(wxPluginId, EvaluationPanel::PluginLocation::PER_BOARD);
    }

    UpdateLogs(res.takeLogs());
}

void CalibrationStageController::OnBoardAddPlugin(const wxCommandEvent& event)
{
    BoardAddPlugin(event);
}

//
void CalibrationStageController::BoardRemovePlugin(const wxCommandEvent& event)
{
    const wxString& wxPluginId = event.GetString();
    const std::string pluginId = wxPluginId.ToStdString();

    model_->unregisterBoardPlugin(pluginId);

    view_->RemovePlugin(wxPluginId);

    view_->EnablePluginPreview(wxPluginId);
}

void CalibrationStageController::OnBoardRemovePlugin(const wxCommandEvent& event)
{
    BoardRemovePlugin(event);
}

//
void CalibrationStageController::doLoadBoardPlugins(const std::string& filePath)
{
    TaskResultP<std::vector<BoardEvaluationEngine::PluginDTO>> res = utils_->loadEvaluationPlugins(filePath);

    UpdateLogs(res.takeLogs());

    if (res.isSuccess())
    {
        settings_->setBoardPluginsPath(filePath);

        view_->RemoveAllPlugins(EvaluationPanel::PluginLocation::PER_BOARD);

        TaskResult loadRes = model_->loadBoardPlugins(res.getPayload());

        view_->SetPluginsPreview(model_->getAvailableBoardPlugins(), EvaluationPanel::PluginLocation::PER_BOARD);

        AppendPlugins(model_->getBoardPlugins(), EvaluationPanel::PluginLocation::PER_BOARD);

        UpdateLogs(loadRes.takeLogs());
    }
}

void CalibrationStageController::LoadBoardPlugins()
{
    wxString message = "Open .json File";
    wxString wildCard = "JSON files (*.json;*.txt)|*.json;*.txt";

    std::optional<wxString> wxFilePath = view_->OpenFileDialog(message, wildCard);
    if (!wxFilePath.has_value())
        return; // user canceled save dialog

    doLoadBoardPlugins(wxFilePath.value().ToStdString());
}

void CalibrationStageController::OnLoadBoardPlugins(const wxCommandEvent& event)
{
    LoadBoardPlugins();
}

//
void CalibrationStageController::doSaveBoardPlugins(const std::string& filePath)
{
    std::vector<std::shared_ptr<PluginContextInfo>> plugins = model_->getBoardPlugins();

    TaskResult res = utils_->saveEvaluationPlugins(filePath, plugins);

    if (res.isSuccess())
    {
        settings_->setBoardPluginsPath(filePath);

        view_->RemoveAllPlugins(EvaluationPanel::PluginLocation::PER_BOARD);

        AppendPlugins(plugins, EvaluationPanel::PluginLocation::PER_BOARD);
    }

    UpdateLogs(res.takeLogs());
}

void CalibrationStageController::SaveBoardPlugins()
{
    wxString message = "Open .json File";
    wxString wildCard = "JSON files (*.json;*.txt)|*.json;*.txt";

    std::optional<wxString> wxFilePath = view_->OpenFileDialog(message, wildCard);
    if (!wxFilePath.has_value())
        return; // user canceled save dialog

    doSaveBoardPlugins(wxFilePath.value().ToStdString());
}

void CalibrationStageController::OnSaveBoardPlugins(const wxCommandEvent& event)
{
    SaveBoardPlugins();
}

//
void CalibrationStageController::BoardSequenceAddPlugin(const wxCommandEvent& event)
{
    const wxString& wxPluginId = event.GetString();
    const std::string pluginId = wxPluginId.ToStdString();

    TaskResult res = model_->registerSequencePlugin(pluginId);

    if (res.isSuccess())
    {
        std::shared_ptr<PluginContextInfo> ctx = model_->getPluginContext(pluginId);
        if (!ctx)
            return;

        view_->AppendPlugin(ctx, EvaluationPanel::PluginLocation::PER_SEQUENCE);
        view_->DisablePluginPreview(wxPluginId, EvaluationPanel::PluginLocation::PER_SEQUENCE);
    }

    UpdateLogs(res.takeLogs());
}

void CalibrationStageController::OnBoardSequenceAddPlugin(const wxCommandEvent& event)
{
    BoardSequenceAddPlugin(event);
}

//
void CalibrationStageController::BoardSequenceRemovePlugin(const wxCommandEvent& event)
{
    const wxString& wxPluginId = event.GetString();
    const std::string pluginId = wxPluginId.ToStdString();

    model_->unregisterSequencePlugin(pluginId);

    view_->RemovePlugin(wxPluginId);

    view_->EnablePluginPreview(wxPluginId);
}

void CalibrationStageController::OnBoardSequenceRemovePlugin(const wxCommandEvent& event)
{
    BoardSequenceRemovePlugin(event);
}

//
void CalibrationStageController::doLoadBoardSequencePlugins(const std::string& filePath)
{
    TaskResultP<std::vector<BoardEvaluationEngine::PluginDTO>> res = utils_->loadEvaluationPlugins(filePath);

    UpdateLogs(res.takeLogs());

    if (res.isSuccess())
    {
        settings_->setSequencePluginsPath(filePath);

        view_->RemoveAllPlugins(EvaluationPanel::PluginLocation::PER_SEQUENCE);

        TaskResult loadRes = model_->loadSequencePlugins(res.getPayload());

        view_->SetPluginsPreview(model_->getAvailableSequencePlugins(), EvaluationPanel::PluginLocation::PER_SEQUENCE);

        AppendPlugins(model_->getSequencePlugins(), EvaluationPanel::PluginLocation::PER_SEQUENCE);

        UpdateLogs(loadRes.takeLogs());
    }
}

void CalibrationStageController::LoadBoardSequencePlugins()
{
    wxString message = "Open .json File";
    wxString wildCard = "JSON files (*.json;*.txt)|*.json;*.txt";

    std::optional<wxString> wxFilePath = view_->OpenFileDialog(message, wildCard);
    if (!wxFilePath.has_value())
        return; // user canceled save dialog

    doLoadBoardSequencePlugins(wxFilePath.value().ToStdString());
}

void CalibrationStageController::OnLoadBoardSequencePlugins(const wxCommandEvent& event)
{
    LoadBoardSequencePlugins();
}

//
void CalibrationStageController::doSaveBoardSequencePlugins(const std::string& filePath)
{
    std::vector<std::shared_ptr<PluginContextInfo>> plugins = model_->getSequencePlugins();

    TaskResult res = utils_->saveEvaluationPlugins(filePath, plugins);

    if (res.isSuccess())
    {
        settings_->setSequencePluginsPath(filePath);

        view_->RemoveAllPlugins(EvaluationPanel::PluginLocation::PER_SEQUENCE);

        AppendPlugins(plugins, EvaluationPanel::PluginLocation::PER_SEQUENCE);
    }

    UpdateLogs(res.takeLogs());
}

void CalibrationStageController::SaveBoardSequencePlugins()
{
    wxString message = "Open .json File";
    wxString wildCard = "JSON files (*.json;*.txt)|*.json;*.txt";

    std::optional<wxString> wxFilePath = view_->OpenFileDialog(message, wildCard);
    if (!wxFilePath.has_value())
        return; // user canceled save dialog

    const std::string filePath = wxFilePath.value().ToStdString();

}

void CalibrationStageController::OnSaveBoardSequencePlugins(const wxCommandEvent& event)
{
    SaveBoardSequencePlugins();
}

//
void CalibrationStageController::ChangePluginThreshold(const wxCommandEvent& event)
{
    const std::string pluginId = event.GetString().ToStdString();
    const double threshold = static_cast<double>(event.GetInt());

    TaskResult res = model_->setPluginThreshold(pluginId, threshold);

    UpdateLogs(res.takeLogs());
}

void CalibrationStageController::OnChangePluginThreshold(const wxCommandEvent& event)
{
    ChangePluginThreshold(event);
}

//
void CalibrationStageController::ChangeGlobalBoardThreshold(const wxCommandEvent& event)
{
    const double threshold = static_cast<double>(event.GetInt());

    view_->UpdateAllPluginsThreshold(threshold, EvaluationPanel::PluginLocation::PER_BOARD);

    model_->setGlobalBoardPluginsThreshold(threshold);
}

void CalibrationStageController::OnChangeGlobalBoardThreshold(const wxCommandEvent& event)
{
    ChangeGlobalBoardThreshold(event);
}

//
void CalibrationStageController::ChangeGlobalBoardSeqThreshold(const wxCommandEvent& event)
{
    const double threshold = static_cast<double>(event.GetInt());

    view_->UpdateAllPluginsThreshold(threshold, EvaluationPanel::PluginLocation::PER_SEQUENCE);

    model_->setGlobalSequencePluginsThreshold(threshold);
}

void CalibrationStageController::OnChangeGlobalBoardSeqThreshold(const wxCommandEvent& event)
{
    ChangeGlobalBoardSeqThreshold(event);
}

/////////////////////////////////////////////////////

//
void CalibrationStageController::EditParam(const ParameterChangedEvent& event)
{
    const ParameterChangedEvent::Value& value = event.GetValue();
    const std::string parameterId = event.GetParameterId().ToStdString();
    const std::string categoryId = event.GetCategoryId().ToStdString();

    std::shared_ptr<ParameterInfo> param = model_->getParameter(parameterId, categoryId);
    if (!param) return;

    TaskResult setRes;
    if (auto* stringVal = std::get_if<wxString>(&value))
    {
        setRes = model_->setParameter(
            parameterId,
            categoryId,
            (*stringVal).ToStdString()
        );
    }
    else if (auto* intVal = std::get_if<int>(&value))
    {
        setRes = model_->setParameter(
            parameterId,
            categoryId,
            *intVal
        );
    }
    else if (auto* doubleVal = std::get_if<double>(&value))
    {
        setRes = model_->setParameter(
            parameterId,
            categoryId,
            *doubleVal
        );
    }
    else if (auto* boolVal = std::get_if<bool>(&value))
    {
        setRes = model_->setParameter(
            parameterId,
            categoryId,
            *boolVal
        );
    }
    else
    {
        setRes = model_->executeCommand(
            parameterId,
            categoryId
        );
    }

    if (!setRes.isSuccess())
    {
        view_->UpdateParameter(param);
    }

    view_->MarkParameterAsDirty(parameterId, model_->isParameterDirty(parameterId, categoryId));

    UpdateLogs(setRes.takeLogs());
}

void CalibrationStageController::OnEditParam(ParameterChangedEvent& event)
{
    EditParam(event);
}

//
void CalibrationStageController::ResetParam(const ParameterEvent& event)
{
    const std::string parameterId = event.GetParameterId().ToStdString();
    const std::string categoryId = event.GetCategoryId().ToStdString();

    std::shared_ptr<ParameterInfo> param = model_->getParameter(parameterId, categoryId);
    if (!param) return;

    TaskResult resetRes = model_->resetParameter(parameterId, categoryId);

    if (resetRes.isSuccess())
    {
        view_->UpdateParameter(param);
        view_->MarkParameterAsDirty(parameterId, model_->isParameterDirty(parameterId, categoryId));
    }

    UpdateLogs(resetRes.takeLogs());
}

void CalibrationStageController::OnResetParam(ParameterEvent& event)
{
    ResetParam(event);
}

//
void CalibrationStageController::EditPluginParam(const ParameterChangedEvent& event)
{
    const ParameterChangedEvent::Value& value = event.GetValue();
    const std::string pluginId = event.GetString().ToStdString();
    const std::string parameterId = event.GetParameterId().ToStdString();
    const std::string categoryId = event.GetCategoryId().ToStdString();

    std::shared_ptr<PluginContextInfo> plugin = model_->getPluginContext(pluginId);
    if (!plugin) return;

    TaskResult setRes;
    if (auto* stringVal = std::get_if<wxString>(&value))
    {
        setRes = model_->setPluginParameter(
            pluginId,
            parameterId,
            categoryId,
            (*stringVal).ToStdString()
        );
    }
    else if (auto* intVal = std::get_if<int>(&value))
    {
        setRes = model_->setPluginParameter(
            pluginId,
            parameterId,
            categoryId,
            *intVal
        );
    }
    else if (auto* doubleVal = std::get_if<double>(&value))
    {
        setRes = model_->setPluginParameter(
            pluginId,
            parameterId,
            categoryId,
            *doubleVal
        );
    }
    else if (auto* boolVal = std::get_if<bool>(&value))
    {
        setRes = model_->setPluginParameter(
            pluginId,
            parameterId,
            categoryId,
            *boolVal
        );
    }
    else
    {
        // TODO: plugin command
    }

    if (!setRes.isSuccess())
    {
        view_->UpdatePlugin(plugin);
    }

    view_->MarkPluginParameterAsDirty(pluginId, parameterId, plugin->isParameterDirty(parameterId, categoryId));
}

void CalibrationStageController::OnEditPluginParam(ParameterChangedEvent& event)
{
    EditPluginParam(event);
}

//
void CalibrationStageController::ResetPluginParam(const ParameterEvent& event)
{
    const std::string pluginId = event.GetString().ToStdString();
    const std::string parameterId = event.GetParameterId().ToStdString();
    const std::string categoryId = event.GetCategoryId().ToStdString();

    std::shared_ptr<PluginContextInfo> plugin = model_->getPluginContext(pluginId);
    if (!plugin) return;

    TaskResult resetRes = model_->resetPluginParameter(pluginId, parameterId, categoryId);

    if (resetRes.isSuccess())
    {
        view_->UpdatePlugin(plugin);
        view_->MarkPluginParameterAsDirty(pluginId, parameterId, plugin->isParameterDirty(parameterId, categoryId));
    }

    UpdateLogs(resetRes.takeLogs());
}

void CalibrationStageController::OnResetPluginParam(ParameterEvent& event)
{
    ResetPluginParam(event);
}

//
void CalibrationStageController::OnSelectBoard(BoardEvent& event)
{
    SelectBoard(event);
}

//
void CalibrationStageController::EnablePlugin(const wxCommandEvent& event)
{
    const std::string pluginId = event.GetString().ToStdString();
    const bool isEnabled = static_cast<bool>(event.GetInt());

    TaskResult res = model_->enablePlugin(pluginId, isEnabled);

    if (!isEnabled)
    {
        view_->ClearPlugin(pluginId);
    }

    UpdateLogs(res.takeLogs());
}

void CalibrationStageController::OnEnablePlugin(wxCommandEvent& event)
{
    EnablePlugin(event);
}
