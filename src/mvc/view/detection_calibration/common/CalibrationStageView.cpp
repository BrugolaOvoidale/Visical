#include "CalibrationStageView.hpp"
#include <wx/checkbox.h>
#include <wx/notebook.h>
#include <gui_elements/image_panel/ImagePanel.hpp>
#include "board/BoardWidgetList.hpp"
#include "board/BoardWidgetDTO.hpp"
#include "board/BoardWidget.hpp"
#include "CalibrationStageViewEvents.hpp"


static const wxString SELECTED_PAGE{ wxT("• ") };

///////////////////////////////////////////////////////////////////////

CalibrationStageView::CalibrationStageView(
    wxWindow* parent,
    wxWindowID winid,
    const wxPoint& pos,
    const wxSize& size,
    long style,
    const wxString& name)
    : BaseView(this), wxPanel(parent, winid, pos, size, style, name)
{
}

///////////////////////////////////////////////////////////////////////

void CalibrationStageView::UpdateImageDisplay(const wxImage& image)
{
    m_imagePanel->SetImage(image);
}

void CalibrationStageView::SetPixelToolTip(const wxString& tip)
{
    m_imagePanel->SetPixelToolTip(tip);
}

void CalibrationStageView::UnsetPixelToolTip()
{
    m_imagePanel->UnsetPixelToolTip();
}

void CalibrationStageView::ClearImageDisplay()
{
    m_imagePanel->ClearDisplay();
}

void CalibrationStageView::DrawBoard(bool draw)
{
    m_drawBoard->SetValue(draw);
}

bool CalibrationStageView::IsDrawBoardEnabled() const
{
    return m_drawBoard->GetValue();
}

void CalibrationStageView::DrawMarks(bool draw)
{
    m_drawMarks->SetValue(draw);
}

bool CalibrationStageView::IsDrawMarksEnabled() const
{
    return m_drawMarks->GetValue();
}

void CalibrationStageView::DrawWCS(bool draw)
{
    m_drawWCS->SetValue(draw);
}

bool CalibrationStageView::IsDrawWCSEnabled() const
{
    return m_drawWCS->GetValue();
}

void CalibrationStageView::UnselectBoard()
{
    m_boardsList->UnselectWidget();
}

bool CalibrationStageView::IsBoardSelected() const
{
    return m_boardsList->GetSelectedBoardId().has_value();
}

std::optional<std::uint32_t> CalibrationStageView::GetSelectedBoard() const
{
    return m_boardsList->GetSelectedBoardId();
}

bool CalibrationStageView::UpdateBoard(
    const BoardWidgetDTO& boardDTO,
    bool needAttention)
{
	const std::uint32_t boardId = boardDTO.GetMapId();

	std::shared_ptr<BoardWidget> widget = m_boardsList->GetWidget(boardId);
    if (!widget)
        return false;

    widget->Update(boardDTO);

    if (needAttention && boardId != m_attentionBoard)
    {
        if (m_attentionBoard) m_boardsList->GetWidget(m_attentionBoard)->SetAttention(false);

        widget->SetAttention(true);

        m_attentionBoard = boardId;
    }

    auto it = m_renderedDataset.find(boardId);
    if (it != m_renderedDataset.end())
    {
        RemoveBoardContribution(it->second);
    }

    m_renderedDataset.emplace(boardId, boardDTO);

    AddBoardContribution(boardDTO);

    DatasetUpdated();

    return true;
}

void CalibrationStageView::AppendBoard(
    const BoardWidgetDTO& boardDTO,
    bool needAttention)
{
    const std::uint32_t boardId = boardDTO.GetMapId();

    auto it = m_renderedDataset.find(boardId);
    if (it != m_renderedDataset.end())
    {
        RemoveBoardContribution(it->second);
    }
    else
    {
        m_datasetStatus.total++;   // new board
    }

    m_renderedDataset.emplace(boardId, boardDTO);

    AddBoardContribution(boardDTO);

    DatasetUpdated();

    const bool appendSuccess = m_boardsList->AppendBoard(boardDTO);

    if (needAttention && appendSuccess)
    {
        if (m_attentionBoard) m_boardsList->GetWidget(m_attentionBoard)->SetAttention(false);

        m_boardsList->GetWidget(boardId)->SetAttention(true);

        m_attentionBoard = boardId;
    }
}

void CalibrationStageView::SelectBoard(std::uint32_t id)
{
    m_boardsList->SelectBoard(id);
}

void CalibrationStageView::RemoveBoard(std::uint32_t id)
{
    m_boardsList->RemoveBoard(id);

    if (id == m_attentionBoard)
    {
        m_attentionBoard = 0;
    }

    auto it = m_renderedDataset.find(id);
    if (it == m_renderedDataset.end())
        return;

    m_datasetStatus.total -= 1;

    const BoardWidgetDTO& dto = it->second;

    {
        const BoardWidgetDTO::BoardStatus oldBoardStatus = dto.GetBoardStatus();
        if (oldBoardStatus == BoardWidgetDTO::BoardStatus::DETECTED)
            m_datasetStatus.detected -= 1;
    }

    {
        const BoardWidgetDTO::EvaluationStatus oldEvalStatus = dto.GetEvaluationCode();

        switch (oldEvalStatus)
        {
            case BoardWidgetDTO::EvaluationStatus::GOOD:
                m_datasetStatus.good -= 1;

                break;

            case BoardWidgetDTO::EvaluationStatus::HAS_ISSUES:
                m_datasetStatus.haveIssues -= 1;

                break;

            default:
                break;
        }
    }

	m_renderedDataset.erase(id);

    DatasetUpdated();
}

void CalibrationStageView::RemoveAllBoards()
{
    m_boardsList->RemoveAllWidgets();

    m_renderedDataset.clear();

    m_datasetStatus.Clear();

    m_attentionBoard = 0;

    DatasetUpdated();
}

bool CalibrationStageView::IsShowDebPluginSelected() const
{
    return m_evaluationPanel->IsShowDebPluginSelected();
}

bool CalibrationStageView::SetSelectedDebPlugin(
    const wxString& pluginId,
    EvaluationPanel::PluginLocation where)
{
    return m_evaluationPanel->SetSelectedDebPlugin(pluginId, where);
}

void CalibrationStageView::MarkPluginParameterAsDirty(
    const wxString& pluginId,
    const wxString& paramId,
    const wxString& categoryId,
    bool isDirty)
{
    m_evaluationPanel->MarkPluginParameterAsDirty(pluginId, paramId, categoryId, isDirty);
}

std::optional<std::pair<wxString, EvaluationPanel::PluginLocation>> CalibrationStageView::GetSelectedDebPlugin() const
{
    return m_evaluationPanel->GetSelectedDebPlugin();
}

void CalibrationStageView::EnablePlugin(
    const wxString& pluginId,
    bool enable)
{
    m_evaluationPanel->EnablePlugin(pluginId, enable);
}

void CalibrationStageView::UpdatePlugin(const std::shared_ptr<PluginContextInfo>& pluginCtx)
{
    m_evaluationPanel->UpdatePlugin(pluginCtx);
}

void CalibrationStageView::UpdatePluginResult(const std::shared_ptr<PluginResult>& pluginResult)
{
    m_evaluationPanel->UpdatePluginResult(pluginResult);
}

void CalibrationStageView::UpdateAllPluginsThreshold(
    double threshold,
    EvaluationPanel::PluginLocation where)
{
    m_evaluationPanel->UpdateAllPluginsThreshold(threshold, where);
}

void CalibrationStageView::AppendPlugin(
    const std::shared_ptr<PluginContextInfo>& pluginCtx,
    EvaluationPanel::PluginLocation where)
{
    m_evaluationPanel->AppendPlugin(pluginCtx, where);
}

void CalibrationStageView::EnableAllPluginsPreview(EvaluationPanel::PluginLocation where)
{
    m_evaluationPanel->EnableAllPluginsPreview(where);
}

void CalibrationStageView::EnablePluginPreview(const wxString& pluginId)
{
    m_evaluationPanel->EnablePluginPreview(pluginId);
}

void CalibrationStageView::DisablePluginPreview(
    const wxString& pluginId,
    EvaluationPanel::PluginLocation where)
{
    m_evaluationPanel->DisablePluginPreview(pluginId, where);
}

void CalibrationStageView::SetPluginsPreview(
    const std::vector<std::shared_ptr<PluginContextInfo>>& plugins,
    EvaluationPanel::PluginLocation where)
{
    m_evaluationPanel->SetPluginsPreview(plugins, where);
}

void CalibrationStageView::ClearPlugin(const wxString& toClearId)
{
	m_evaluationPanel->ClearPlugin(toClearId);
}

void CalibrationStageView::ClearAllPlugins(EvaluationPanel::PluginLocation where)
{
    m_evaluationPanel->ClearAllPlugins(where);
}

void CalibrationStageView::RemovePlugin(const wxString& toRemoveId)
{
    m_evaluationPanel->RemovePlugin(toRemoveId);
}

void CalibrationStageView::RemoveAllPlugins(EvaluationPanel::PluginLocation where)
{
    m_evaluationPanel->RemoveAllPlugins(where);
}

///////////////////////////////////////////////////////////////////////

void CalibrationStageView::RemoveBoardContribution(const BoardWidgetDTO& dto)
{
    if (dto.GetBoardStatus() == BoardWidgetDTO::BoardStatus::DETECTED)
        m_datasetStatus.detected--;

    switch (dto.GetEvaluationCode())
    {
        case BoardWidgetDTO::EvaluationStatus::GOOD:
            m_datasetStatus.good--;
            break;

        case BoardWidgetDTO::EvaluationStatus::HAS_ISSUES:
            m_datasetStatus.haveIssues--;
            break;

        default:
            break;
    }
}

void CalibrationStageView::AddBoardContribution(const BoardWidgetDTO& dto)
{
    if (dto.GetBoardStatus() == BoardWidgetDTO::BoardStatus::DETECTED)
        m_datasetStatus.detected++;

    switch (dto.GetEvaluationCode())
    {
        case BoardWidgetDTO::EvaluationStatus::GOOD:
            m_datasetStatus.good++;
            break;

        case BoardWidgetDTO::EvaluationStatus::HAS_ISSUES:
            m_datasetStatus.haveIssues++;
            break;

        default:
            break;
    }
}

void CalibrationStageView::OnPageChanged(wxBookCtrlEvent& event)
{
    {
        wxString newTxt = m_notebook->GetPageText(m_currPage);
        newTxt.Replace(SELECTED_PAGE, wxEmptyString, false);

        m_notebook->SetPageText(m_currPage, newTxt);
    }

    {
        int sel = event.GetSelection();

        m_currPage = sel;

        wxString newTxt = m_notebook->GetPageText(m_currPage);
        newTxt.Prepend(SELECTED_PAGE);

        m_notebook->SetPageText(m_currPage, newTxt);
    }
}

void CalibrationStageView::OnLoadSettings(wxCommandEvent& event)
{
    // Let the parent know this board was clicked
    wxCommandEvent evt(GUI_LOAD_SETTINGS, GetId());

    // Send it to parent
    ProcessEvent(evt);         // sends to this and upwards
}

void CalibrationStageView::OnSaveSettings(wxCommandEvent& event)
{
    // Let the parent know this board was clicked
    wxCommandEvent evt(GUI_SAVE_SETTINGS, GetId());

    // Send it to parent
    ProcessEvent(evt);         // sends to this and upwards
}

void CalibrationStageView::OnLoadModelConfig(wxCommandEvent& event)
{
    // Let the parent know this board was clicked
    wxCommandEvent evt(GUI_LOAD_MODEL_PARAMS, GetId());

    // Send it to parent
    ProcessEvent(evt);         // sends to this and upwards
}

void CalibrationStageView::OnSaveModelConfig(wxCommandEvent& event)
{
    // Let the parent know this board was clicked
    wxCommandEvent evt(GUI_SAVE_MODEL_PARAMS, GetId());

    // Send it to parent
    ProcessEvent(evt);         // sends to this and upwards
}

void CalibrationStageView::OnDrawBoard(wxCommandEvent& event)
{
    wxCommandEvent evt(GUI_DRAW_BOARD, GetId());

    evt.SetInt(m_drawBoard->IsChecked());

    ProcessEvent(evt);

    event.Skip();
}

void CalibrationStageView::OnDrawMarks(wxCommandEvent& event)
{
    wxCommandEvent evt(GUI_DRAW_MARKS, GetId());

    evt.SetInt(m_drawMarks->IsChecked());

    ProcessEvent(evt);

    event.Skip();
}

void CalibrationStageView::OnDrawWCS(wxCommandEvent& event)
{
    wxCommandEvent evt(GUI_DRAW_WCS, GetId());

    evt.SetInt(m_drawWCS->IsChecked());

    ProcessEvent(evt);

    event.Skip();
}

void CalibrationStageView::OnClose(wxEvent& event)
{
    Hide();
}