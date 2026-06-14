#include "LoggerView.hpp"
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/filename.h>
#include <wx/image.h>
#include <wx/button.h>
#include <wx/dataview.h>
#include <wx/display.h>
#include <chrono>
#include <UtilityFunctions.hpp>
#include <logger/Log.hpp>
#include <message_bus/types/MessageTask.hpp>


///////////////////////////////////////////////////////////////////////////

wxDEFINE_EVENT(EVT_SELECT_MESSAGE, wxCommandEvent);

///////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////

LoggerView::LoggerView(
	wxWindow* parent,
	wxWindowID id,
	const wxString& title,
	bool startShowed,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxString& name)
	: wxFrame(parent, id, title, pos, size, style, name)
{
	wxIcon icon;
	icon.LoadFile("resources/appIcon.ico", wxBITMAP_TYPE_ICO);
	SetIcon(icon);

	wxPanel* mainPanel = new wxPanel(this);

	// Logs list
	m_logsList = CreateLogsList(mainPanel);

	wxButton* clearLogButton = new wxButton(mainPanel, wxID_ANY, "Clear Logs");
	clearLogButton->Bind(wxEVT_BUTTON, &LoggerView::OnClearLog, this);

	wxBoxSizer* buttonsSizer = new wxBoxSizer(wxHORIZONTAL);
	buttonsSizer->AddStretchSpacer(1);
	buttonsSizer->Add(clearLogButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 0);


	// Messages list
	wxBoxSizer* messageSizer = new wxBoxSizer(wxHORIZONTAL);

	m_messagesList = CreateMessagesList(mainPanel);
	m_messagesList->Bind(wxEVT_DATAVIEW_SELECTION_CHANGED, &LoggerView::OnSelectMessage, this);

	messageSizer->Add(m_messagesList, 1, wxEXPAND | wxALL, 0);

	wxBoxSizer* additionalSizer = new wxBoxSizer(wxVERTICAL);

	m_msgLogsList = CreateLogsList(mainPanel);

	additionalSizer->Add(m_msgLogsList, 1, wxEXPAND | wxALL, 0);

	m_msgCtxList = CreateMessageContextList(mainPanel);

	additionalSizer->Add(m_msgCtxList, 1, wxEXPAND | wxALL, 0);

	messageSizer->Add(additionalSizer, 1, wxEXPAND | wxALL, 0);

	wxButton* clearMsgButton = new wxButton(mainPanel, wxID_ANY, "Clear Messages");
	clearMsgButton->Bind(wxEVT_BUTTON, &LoggerView::OnClearMessages, this);

	wxBoxSizer* msgButtonsSizer = new wxBoxSizer(wxHORIZONTAL);
	msgButtonsSizer->AddStretchSpacer(1);
	msgButtonsSizer->Add(clearMsgButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 0);


	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
	mainSizer->Add(m_logsList, 1, wxEXPAND | wxALL, 0);
	mainSizer->Add(buttonsSizer, 0, wxEXPAND | wxALL, 0);
	mainSizer->Add(messageSizer, 1, wxEXPAND | wxALL, 0);
	mainSizer->Add(msgButtonsSizer, 0, wxEXPAND | wxALL, 0);
	mainPanel->SetSizerAndFit(mainSizer);
	mainPanel->SetMinSize(mainPanel->GetSize());

	// DPI-aware minimum for the whole frame
	const wxSize kMinFrameSize = FromDIP(mainSizer->GetMinSize());
	SetMinClientSize(kMinFrameSize);

	wxDisplay display(wxDisplay::GetFromWindow(this));
	wxRect screenRect = display.GetClientArea();

	// Initial size: 80%x80% of screen, but never smaller than our minimum
	const int width = std::max((int)(screenRect.GetWidth() * 0.8), kMinFrameSize.x);
	const int height = std::max((int)(screenRect.GetHeight() * 0.8), kMinFrameSize.y);
	SetSize(wxSize(width, height));

	Centre();

	Bind(wxEVT_CLOSE_WINDOW, &LoggerView::OnClose, this);

	Layout();

	Show(startShowed);
}

///////////////////////////////////////////////////////////////////////////

void LoggerView::Open()
{
	if (!IsIconized())  Restore();

	if (!IsShown())     Show();

	if (!HasFocus())    SetFocus();

	Centre();
}

void LoggerView::UpdateLogs(const Log& logs)
{
	DoUpdateLogs(logs, m_logsList);
}

void LoggerView::UpdateLogs(const std::vector<Log>& logs)
{
	if (logs.empty())
		return;

	for (const auto& l : logs)
	{
		UpdateLogs(l);
	}
}

void LoggerView::UpdateMessage(const Message& msg)
{
	wxVector<wxVariant> row;

	// Time
	row.push_back(FormatTimePoint(msg.timestamp()));

	// Tag
	row.push_back(std::string(msg.tag().name()));

	// Source
	row.push_back(wxString::Format("%d", msg.sourceId().get()));

	// Status
	row.push_back("---");

	m_messagesList->AppendItem(row);
}

void LoggerView::UpdateMessage(const MessageTask& msg)
{
	wxVector<wxVariant> row;

	// Time
	row.push_back(FormatTimePoint(msg.timestamp()));

	// Tag
	row.push_back(std::string(msg.tag().name()));

	// Source
	row.push_back(wxString::Format("%d", msg.sourceId().get()));

	// Status
	row.push_back(UtilityFunctions::stringFromEnum(msg.status()).value());

	m_messagesList->AppendItem(row);
}

void LoggerView::UpdateMessageLogs(const std::vector<Log>& logs)
{
	if (logs.empty())
		return;

	for (const auto& l : logs)
	{
		DoUpdateLogs(l, m_msgLogsList);
	}
}

void LoggerView::UpdateMessageContext(const MessageContext& msgCtx)
{
	const std::unordered_map<std::string, MessageContext::Attribute>& attributes = msgCtx.getAttributes();

	for (const auto& [key, value] : attributes)
	{
		wxVector<wxVariant> row;

		// Key
		row.push_back(key);

		// Attribute
		std::visit(
			[&row](auto&& val)
			{
				using T = std::decay_t<decltype(val)>;
				if constexpr (std::is_same_v<T, std::int64_t>)
				{
					row.push_back(static_cast<int>(val));
				}
				else if constexpr (std::is_same_v<T, double>)
				{
					row.push_back(val);
				}
				else if constexpr (std::is_same_v<T, bool>)
				{
					row.push_back(val);
				}
				else if constexpr (std::is_same_v<T, std::string>)
				{
					row.push_back(val);
				}
			}
		, value);
		
		m_msgCtxList->AppendItem(row);
	}
}

///////////////////////////////////////////////////////////////////////////

wxDataViewListCtrl* LoggerView::CreateLogsList(wxWindow* parent)
{
	wxDataViewListCtrl* list = new wxDataViewListCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxDV_ROW_LINES | wxDV_VERT_RULES);

	list->AppendTextColumn("Time", wxDATAVIEW_CELL_INERT, 140, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE);
	list->AppendTextColumn("Level", wxDATAVIEW_CELL_INERT, 80, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE);
	list->AppendTextColumn("What", wxDATAVIEW_CELL_INERT, 120, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE);
	list->AppendTextColumn("Who", wxDATAVIEW_CELL_INERT, 200, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE);
	list->AppendTextColumn("Where", wxDATAVIEW_CELL_INERT, 400, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE);

	return list;
}

wxDataViewListCtrl* LoggerView::CreateMessagesList(wxWindow* parent)
{
	wxDataViewListCtrl* list = new wxDataViewListCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxDV_ROW_LINES | wxDV_VERT_RULES);

	list->AppendTextColumn("Time", wxDATAVIEW_CELL_INERT, 140, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE);
	list->AppendTextColumn("Tag", wxDATAVIEW_CELL_INERT, 80, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE);
	list->AppendTextColumn("Source", wxDATAVIEW_CELL_INERT, 120, wxALIGN_CENTER, wxDATAVIEW_COL_RESIZABLE);
	list->AppendTextColumn("Status", wxDATAVIEW_CELL_INERT, 200, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE);

	return list;
}

wxDataViewListCtrl* LoggerView::CreateMessageContextList(wxWindow* parent)
{
	wxDataViewListCtrl* list = new wxDataViewListCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxDV_ROW_LINES | wxDV_VERT_RULES);

	list->AppendTextColumn("Key", wxDATAVIEW_CELL_INERT, 140, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE);
	list->AppendTextColumn("Attribute", wxDATAVIEW_CELL_INERT, 80, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE);

	return list;
}

void LoggerView::DoUpdateLogs(
	const Log& logs,
	wxDataViewListCtrl* list)
{
	wxString whereStr = wxFileName(logs.where().file()).GetFullName()
		+ ":" + wxString::Format("%d", logs.where().line());

	wxString whoStr = wxString(logs.who()).BeforeFirst('(').AfterLast(' ');

	wxVector<wxVariant> row;

	// Time
	row.push_back(FormatTimePoint(logs.when()));

	// Level
	row.push_back(UtilityFunctions::stringFromEnum(logs.level()).value());

	// What
	row.push_back(logs.what());

	// Who
	row.push_back(whoStr);

	// Where
	row.push_back(whereStr);

	list->AppendItem(row);
}

void LoggerView::OnClearLog(wxCommandEvent& event)
{
	m_logsList->DeleteAllItems();
}

void LoggerView::OnClearMessages(wxCommandEvent& event)
{
	m_messagesList->DeleteAllItems();
	m_msgLogsList->DeleteAllItems();
	m_msgCtxList->DeleteAllItems();
}

void LoggerView::OnSelectMessage(wxDataViewEvent& event)
{
	wxDataViewItem item = event.GetItem();
	if (!item.IsOk())
		return;

	m_msgLogsList->DeleteAllItems();
	m_msgCtxList->DeleteAllItems();

	unsigned int row = m_messagesList->ItemToRow(item);

	wxCommandEvent evt(EVT_SELECT_MESSAGE, GetId());

	evt.SetInt(row);

	ProcessEvent(evt);
}

void LoggerView::OnClose(wxCloseEvent& event)
{
	if (event.CanVeto())
	{
		event.Veto();

		Hide();

		return;
	}

	event.Skip(); // allow normal destruction
}
