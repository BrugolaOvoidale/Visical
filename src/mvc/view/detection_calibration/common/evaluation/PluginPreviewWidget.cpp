#include "PluginPreviewWidget.hpp"
#include <wx/image.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <gui_elements/persistent_tooltip/PersistentToolTip.hpp>
#include <evaluator/PluginContextInfo.hpp>
#include <evaluator/PluginInfo.hpp>
#include "PluginPreviewWidgetEvents.hpp"


static const wxString PLUS_ICON{ "resources/plus.png" };
static const wxString MINUS_ICON{ "resources/minus.png" };

///////////////////////////////////////////////

PluginPreviewWidget::PluginPreviewWidget(
    wxWindow* parent,
    const std::shared_ptr<PluginContextInfo>& pluginCtx)
    : BaseWidget(parent, false),
    m_pluginId(pluginCtx->getPluginInfo()->id())
{
    m_addBitmap = wxImage(PLUS_ICON, wxBITMAP_TYPE_PNG).Rescale(18, 18);

    m_removeBitmap = wxImage(MINUS_ICON, wxBITMAP_TYPE_PNG).Rescale(18, 18);

	Update(pluginCtx);
}

//////////////////////////////////////////////////////////////////////////

wxString PluginPreviewWidget::GetPluginId() const
{
    return m_pluginId;
}

void PluginPreviewWidget::Update(const std::shared_ptr<PluginContextInfo>& pluginCtx)
{
    const std::shared_ptr<PluginInfo>& plugin = pluginCtx->getPluginInfo();

    if (!pluginCtx || plugin->id() != m_pluginId)
        return;

    SetDescription(plugin->description());

    SetNote(plugin->name());
}

void PluginPreviewWidget::EnablePlugin(bool enable)
{
    if (m_isEnabled == enable)
        return;

    m_isEnabled = enable;

    if (IsInitialized())
        DoEnable();
}

void PluginPreviewWidget::SetNote(const wxString& newNote)
{
    if (m_note == newNote)
        return;

    m_note = newNote;

    if (IsInitialized())
        DoSetNote();
}

const wxString& PluginPreviewWidget::GetNote() const
{
    return m_note;
}


//////////////////////////////////////////////////////////////////////////

wxPanel* PluginPreviewWidget::CreateHeader()
{
    // --- Header row ---
    wxPanel* header = new wxPanel(m_widgetPanel);

    wxBoxSizer* headerSizer = new wxBoxSizer(wxHORIZONTAL);

    // Note text
    m_noteItem = new wxStaticText(
        header,
        wxID_ANY,
        wxEmptyString,
        wxDefaultPosition,
        wxDefaultSize,
        wxST_ELLIPSIZE_MIDDLE
    );
    m_noteItem->SetMinSize(m_widgetPanel->FromDIP(wxSize(150, -1)));
    BindSelectable(m_noteItem);
    headerSizer->Add(m_noteItem, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);


    // Stretch spacer pushes threshold to the right edge
    headerSizer->AddStretchSpacer();

    m_addBtn = new wxBitmapButton(header, wxID_ANY, m_addBitmap);
    m_addBtn->SetMinSize(m_widgetPanel->FromDIP(m_addBtn->GetBestSize()));
    m_addBtn->Bind(wxEVT_BUTTON, &PluginPreviewWidget::OnAddRemovePlugin, this);
    BindSelectable(m_addBtn, true);

    headerSizer->Add(m_addBtn, 0, wxALIGN_CENTER_VERTICAL);

    header->SetSizer(headerSizer);

    return header;
}

void PluginPreviewWidget::InitializeImpl()
{
    DoSetNote();

    DoEnable();
}

void PluginPreviewWidget::SetDescriptionImpl()
{
    PersistentToolTip::SetToolTip(m_noteItem, m_description);
}

void PluginPreviewWidget::DoSetNote()
{
    m_noteItem->SetLabel(m_note);
}

void PluginPreviewWidget::DoEnable()
{
    if (!m_isEnabled)
    {
        m_addBtn->SetBitmap(m_removeBitmap);

        m_addBtn->SetToolTip("Remove plugin");
    }
    else
    {

        m_addBtn->SetBitmap(m_addBitmap);

        m_addBtn->SetToolTip("Add plugin");
    }
}

void PluginPreviewWidget::OnSpecializedWidgetClick(wxMouseEvent& event)
{
    // Let the parent know this plugin was clicked
    wxCommandEvent evt(GUI_SELECT_PLUGIN_PREVIEW, m_widgetPanel->GetId());
    evt.SetClientData(wxUIntToPtr(GetWidgetId()));
    evt.SetString(GetPluginId());

    // Send it to parent
    ProcessEvent(evt);         // sends to this and upwards

    event.Skip();
}

void PluginPreviewWidget::OnAddRemovePlugin(wxCommandEvent& event)
{
    const wxEventType type = m_isEnabled ? GUI_ADD_PLUGIN : GUI_REMOVE_PLUGIN;

    wxCommandEvent evt(type, m_widgetPanel->GetId());
    evt.SetClientData(wxUIntToPtr(GetWidgetId()));
    evt.SetString(GetPluginId());

    ProcessEvent(evt);

    event.Skip();
}