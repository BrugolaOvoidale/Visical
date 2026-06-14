#include "BoardWidget.hpp"
#include <inttypes.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <gui_elements/persistent_tooltip/PersistentToolTip.hpp>
#include "BoardWidgetEvents.hpp"


static const wxString ATTENTION{ wxT("◄") };
static const wxString BOARD_DETECTED{ "DETECTED" };
static const wxString BOARD_NOT_DETECTED{ "NOT DETECTED" };
static const wxString BOARD_POSE_NOT_SOLVED{ "POSE NOT SOLVED" };
static const wxString BOARD_EVAL_ISSUE{ "EVALUATION ISSUE" };
static const wxString BOARD_NOT_EVAL{ "NOT EVALUATED" };

/////////////////////////////////////////////////////

BoardWidget::BoardWidget(
    wxWindow* parent,
    const BoardWidgetDTO& boardDTO)
    : BaseWidget(parent, true),
    m_boardId(boardDTO.GetMapId())
{
    Update(boardDTO);
}

//////////////////////////////////////////////////////////////////////////

void BoardWidget::Update(const BoardWidgetDTO& boardDTO)
{
    SetNote(boardDTO.GetNote());

    SetStatus(boardDTO.GetBoardStatus(), boardDTO.GetEvaluationCode());

    UpdateMetadata(boardDTO.GetMetadata());
}

std::uint32_t BoardWidget::GetBoardId() const
{
    return m_boardId;
}

void BoardWidget::SetNote(const wxString& newNote)
{
    if (m_note == newNote)
        return;

    m_note = newNote;

    if (IsInitialized())
        DoSetNote();
}

void BoardWidget::SetStatus(
    BoardWidgetDTO::BoardStatus boardStatus,
    BoardWidgetDTO::EvaluationStatus evalStatus)
{
    if (m_boardStatus == boardStatus && m_evalStatus == evalStatus)
        return;

    m_boardStatus = boardStatus;
    m_evalStatus = evalStatus;

    if (IsInitialized())
        DoSetStatus();
}

void BoardWidget::SetAttention(bool needAttention)
{
    if (m_hasAttention == needAttention)
        return;

    m_hasAttention = needAttention;

    if (IsInitialized())
        DoSetAttention();
}

bool BoardWidget::NeedAttention() const
{
    return m_hasAttention;
}

//////////////////////////////////////////////////////////////////////////

wxPanel* BoardWidget::CreateHeader()
{
    // Header row
    wxPanel* header = new wxPanel(m_widgetPanel);
    wxBoxSizer* headerSizer = new wxBoxSizer(wxHORIZONTAL);


    wxStaticText* boardIdText = new wxStaticText(
        header,
        wxID_ANY,
        "Board Id: " + wxString::Format("%" PRIu32, GetBoardId())
    );

    BindSelectable(boardIdText);
    headerSizer->Add(boardIdText, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);

    m_noteItem = new wxStaticText(
        header,
        wxID_ANY,
        wxEmptyString,
        wxDefaultPosition,
        wxSize(100, -1), // Width limits it
        wxST_ELLIPSIZE_MIDDLE // Ellipsize at the midlle
    );
    BindSelectable(m_noteItem);
    headerSizer->Add(m_noteItem, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);

    m_detectionStatus = new wxStaticText(header, wxID_ANY, wxEmptyString);
    BindSelectable(m_detectionStatus);
    headerSizer->Add(m_detectionStatus, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);

    m_evaluationStatus = new wxStaticText(header, wxID_ANY, wxEmptyString);
    BindSelectable(m_evaluationStatus);
    headerSizer->Add(m_evaluationStatus, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);

    headerSizer->AddStretchSpacer(1);  // Pushes the next item to the right


    m_attentionText = new wxStaticText(header, wxID_ANY, ATTENTION);
    m_attentionText->SetToolTip("Last processed board");
    m_attentionText->Hide();
    BindSelectable(m_attentionText);
    headerSizer->Add(m_attentionText, 0, wxALIGN_CENTER_VERTICAL);

    header->SetSizer(headerSizer);

    return header;
}

void BoardWidget::InitializeImpl()
{
    DoSetNote();

    DoSetStatus();

    DoSetAttention();
}

void BoardWidget::SetDescriptionImpl()
{
}

void BoardWidget::DoSetNote()
{
    m_noteItem->SetLabel(m_note);
    PersistentToolTip(m_noteItem, m_note);
}

void BoardWidget::DoSetStatus()
{
    bool isBoardFatal = false;

    // Detection label

    switch (m_boardStatus)
    {
        case BoardWidgetDTO::BoardStatus::NONE:
            m_detectionStatus->SetLabel(wxEmptyString);
            break;

        case BoardWidgetDTO::BoardStatus::DETECTED:
            m_detectionStatus->SetLabel(BOARD_DETECTED);
            break;

        case BoardWidgetDTO::BoardStatus::NOT_DETECTED:
            m_detectionStatus->SetLabel(BOARD_NOT_DETECTED);
            break;

        case BoardWidgetDTO::BoardStatus::POSE_NOT_SOLVED:
            m_detectionStatus->SetLabel(BOARD_POSE_NOT_SOLVED);
            break;
    }

    // Evaluation label

    switch (m_evalStatus)
    {
        case BoardWidgetDTO::EvaluationStatus::GOOD:
            m_evaluationStatus->SetLabel(wxEmptyString);
            break;

        case BoardWidgetDTO::EvaluationStatus::HAS_ISSUES:
            m_evaluationStatus->SetLabel(BOARD_EVAL_ISSUE);
            break;

        case BoardWidgetDTO::EvaluationStatus::NOT_EVALUATED:
            m_evaluationStatus->SetLabel(BOARD_NOT_EVAL);
            break;
    }

    // Colour logic

    wxColour colour = BaseWidget::DEFAULT_GRAY;

    // Detection errors dominate
    if (m_boardStatus == BoardWidgetDTO::BoardStatus::NOT_DETECTED ||
        m_boardStatus == BoardWidgetDTO::BoardStatus::POSE_NOT_SOLVED)
    {
        colour = BaseWidget::HIGHLIGHT_RED;
    }
    // Evaluation issues next
    else if (m_evalStatus == BoardWidgetDTO::EvaluationStatus::HAS_ISSUES)
    {
        colour = BaseWidget::HIGHLIGHT_YELLOW;
    }
    // Successful detection last
    else if (m_evalStatus == BoardWidgetDTO::EvaluationStatus::GOOD ||
        m_boardStatus == BoardWidgetDTO::BoardStatus::DETECTED)
    {
        colour = BaseWidget::HIGHLIGHT_GREEN;
    }

    SetColour(colour);
}

void BoardWidget::DoSetAttention()
{
	m_attentionText->Show(m_hasAttention);

    m_widgetPanel->Layout();
}

void BoardWidget::OnSpecializedWidgetClick(wxMouseEvent& event)
{
    // Let the parent know this board was clicked
    BoardEvent evt(GUI_BOARD_CLICK, m_widgetPanel->GetId());
    evt.SetClientData(wxUIntToPtr(GetWidgetId()));
    evt.SetBoardId(GetBoardId());

    // Send it to parent
    ProcessEvent(evt);         // sends to this and upwards

    event.Skip();
}