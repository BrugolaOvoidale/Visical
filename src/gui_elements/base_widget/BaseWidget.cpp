#include "BaseWidget.hpp"
#include <wx/panel.h>
#include <wx/button.h>
#include <wx/statline.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include "../persistent_tooltip/PersistentToolTip.hpp"


///////////////////////////////////////////////

wxDEFINE_EVENT(GUI_TOGGLE_META_WIDGET, wxCommandEvent);

///////////////////////////////////////////////

static const wxString EXPAND{ wxT("▼") };
static const wxString COLLAPSE{ wxT("▲") };

///////////////////////////////////////////////


BaseWidget::BaseWidget(
    wxWindow* parent,
    bool hasMetaPanel)
    : m_parent(parent),
    m_widgetId(++m_widgetIdCounter),
    m_hasMetadataPanel(hasMetaPanel)
{
    SetNextHandler(m_parent);
}

BaseWidget::~BaseWidget()
{
    Deinitialize();
}

///////////////////////////////////////////////////////////

wxUIntPtr BaseWidget::GetWidgetId() const
{
    return m_widgetId;
}

void BaseWidget::Initialize(bool startShowed)
{
    if (m_initialized)
        return;

    m_widgetPanel = new wxPanel(m_parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE);

    m_widgetPanel->SetBackgroundColour(DEFAULT_GRAY);

    m_rootSizer = new wxBoxSizer(wxVERTICAL);

    // Header
    m_headerSizer = new wxBoxSizer(wxHORIZONTAL);

    if (m_hasMetadataPanel)
    {
        m_toggleBtn = new wxButton(m_widgetPanel, wxID_ANY, EXPAND, wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
        m_toggleBtn->SetMinSize(m_widgetPanel->FromDIP(wxSize(18, 18)));

        wxStaticLine* sep = new wxStaticLine(m_widgetPanel, wxID_ANY, wxDefaultPosition, wxSize(1, 20), wxLI_VERTICAL);

        m_headerSizer->Add(m_toggleBtn, 0, wxALIGN_CENTER_VERTICAL);
        m_headerSizer->Add(sep, 0, wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 10);

        BindSelectable(sep);
        m_toggleBtn->Bind(wxEVT_BUTTON, &BaseWidget::OnToggleMetaPanel, this);
    }

    m_rootSizer->Add(m_headerSizer, 0, wxEXPAND, 8);

    // Metadata panel
    m_metadataPanel = new wxPanel(m_widgetPanel);

    m_metaItemsSizer = new wxBoxSizer(wxVERTICAL);

    m_metaSizer = new wxStaticBoxSizer(wxVERTICAL, m_metadataPanel, "Metadata");

    m_metaItemsSizer->Add(m_metaSizer, 0, wxEXPAND);

    m_metadataPanel->SetSizer(m_metaItemsSizer);

    BindSelectable(m_metadataPanel);

    m_rootSizer->Add(m_metadataPanel, 0, wxEXPAND /* | wxLEFT | wxRIGHT | wxBOTTOM */, 8);

    m_widgetPanel->SetSizer(m_rootSizer);

    m_metadataPanel->Hide();

    m_widgetPanel->Bind(wxEVT_SIZE, &BaseWidget::OnSize, this);
    BindSelectable(m_widgetPanel);

    m_derivedHeader = CreateHeader();
    m_headerSizer->Add(m_derivedHeader, 1, wxEXPAND | wxALL, 8);

    BindSelectable(m_derivedHeader);

    DoSetSelected();

    DoSetDescription();

    DoSetColour();

    DoUpdateMetadataBulk();

    m_initialized = true;

    InitializeImpl();

    m_widgetPanel->Layout();

    m_widgetPanel->Show(startShowed);
}

void BaseWidget::Deinitialize()
{
    if (!m_initialized) return;

    m_widgetPanel->Unbind(wxEVT_SIZE, &BaseWidget::OnSize, this);
    if (m_hasMetadataPanel)
        m_toggleBtn->Unbind(wxEVT_BUTTON, &BaseWidget::OnToggleMetaPanel, this);

    std::unordered_set<wxObject*> bindedElements = m_bindedSelectElements;
    for (wxObject* e : bindedElements)
    {
        UnbindSelectable(wxStaticCast(e, wxWindow));
    }

    // Destroying the root panel cascades to all children.
    m_widgetPanel->Destroy();
    m_widgetPanel = nullptr;

    // Null out all child pointers, they're gone with the parent
    m_rootSizer = nullptr;
    m_headerSizer = nullptr;
    m_toggleBtn = nullptr;
    m_metadataPanel = nullptr;
    m_metaSizer = nullptr;
    m_metaItemsSizer = nullptr;
    m_derivedHeader = nullptr;
    m_currMetadata.clear();
    m_currMetadataWidgets.clear();

    m_initialized = false;
}

wxPanel* BaseWidget::GetWidgetPanel() const
{
    return m_widgetPanel;
}

int BaseWidget::GetPreferredHeight() const
{
    if (IsInitialized())
        return m_widgetPanel->GetBestSize().GetHeight();

    // TODO: I don't like it

    //const_cast<BaseWidget*>(this)->Initialize();
    //int h = m_widgetPanel->GetBestSize().GetHeight();
    //const_cast<BaseWidget*>(this)->Deinitialize();

    return /*h*/ 44;
}

bool BaseWidget::IsInitialized() const
{
    return m_initialized;
}

bool BaseWidget::HasMetadataPanel() const
{
    return m_hasMetadataPanel;
}

void BaseWidget::SetSelected(bool selected)
{
    if (m_isSelected == selected)
        return;

    m_isSelected = selected;

    const wxColour newColour = m_isSelected ? HIGHLIGHT_BLUE : m_normalColour;

    if (m_currColour != newColour)
    {
        m_currColour = newColour;
        
        if (IsInitialized())
            DoSetSelected();
    }
}

bool BaseWidget::IsSelected() const
{
    return m_isSelected;
}

void BaseWidget::SetDescription(const wxString& description)
{
    if (m_description == description) return;

    m_description = description;

    if (IsInitialized())
        DoSetDescription();
}

const wxString& BaseWidget::GetDescription() const
{
    return m_description;
}

void BaseWidget::SetColour(const wxColour& colour)
{
    if (m_normalColour == colour)
        return;

    m_normalColour = colour;

    if (!IsSelected() && m_currColour != colour)
        m_currColour = colour;

    if (IsInitialized())
        DoSetColour();
}

wxColour BaseWidget::GetColour() const
{
    return m_currColour;
}

void BaseWidget::UpdateMetadata(
    const wxString& key,
    const wxString& value)
{
    m_currMetadata[key] = key + ": " + value;

    if (IsInitialized())
		DoUpdateMetadata(key);
}

void BaseWidget::UpdateMetadata(const std::unordered_map<wxString, wxString>& metadata)
{
    // Update internal data first
    m_currMetadata = metadata;

    // Remove keys no longer present
    for (auto it = m_currMetadata.begin(); it != m_currMetadata.end(); )
    {
        if (metadata.find(it->first) == metadata.end())
            it = m_currMetadata.erase(it);
        else
            ++it;
    }

    if (IsInitialized())
        DoUpdateMetadataBulk();
}

///////////////////////////////////////////////////////////

void BaseWidget::AddMetadataItem(wxWindow* item)
{
    m_metaItemsSizer->Insert(m_metaItemsSizer->GetItemCount() - 1, item, 0);
}

void BaseWidget::AddMetadataItem(wxSizer* item)
{
    m_metaItemsSizer->Insert(m_metaItemsSizer->GetItemCount() - 1, item, 0);
}

void BaseWidget::BindSelectable(
    wxWindow* w,
    bool keepSelectedOnClick)
{
    if (keepSelectedOnClick)
        m_keepSelectedOnClick.insert(w);

    m_bindedSelectElements.insert(w);

    w->Bind(wxEVT_LEFT_DOWN, &BaseWidget::OnSpecializedWidgetClick, this);
    w->Bind(wxEVT_LEFT_DOWN, &BaseWidget::OnWidgetClick, this);
}

void BaseWidget::UnbindSelectable(wxWindow* w)
{
    m_bindedSelectElements.erase(w);
    m_keepSelectedOnClick.erase(w);

    w->Unbind(wxEVT_LEFT_DOWN, &BaseWidget::OnSpecializedWidgetClick, this);
    w->Unbind(wxEVT_LEFT_DOWN, &BaseWidget::OnWidgetClick, this);
}

wxColour BaseWidget::GetNormalColour() const
{
    return m_normalColour;
}

void BaseWidget::OnWidgetClick(wxMouseEvent& event)
{
    // Keep selection if clicking on specific child widgets
    wxObject* clickedObject = event.GetEventObject();
    if (IsSelected() && m_keepSelectedOnClick.contains(clickedObject))
    {
        // Maintain current selection
    }
    else
    {
        // Toggle selection state
        SetSelected(!IsSelected());
    }

    event.Skip();
}

void BaseWidget::OnToggleMetaPanel(wxCommandEvent&)
{
    if (!IsInitialized())
        return;

    if (m_metadataPanel->IsShown())
    {
        m_metadataPanel->Hide();
        m_toggleBtn->SetLabel(EXPAND);
    }
    else
    {
        m_metadataPanel->Show();
        m_toggleBtn->SetLabel(COLLAPSE);
    }

    m_widgetPanel->Layout();

    // Let the parent know this camera was toggled
    wxCommandEvent evt(GUI_TOGGLE_META_WIDGET, m_widgetPanel->GetId());
    evt.SetInt(m_metadataPanel->IsShown());
    evt.SetClientData(wxUIntToPtr(GetWidgetId()));

    // Send it to parent
    ProcessEvent(evt);  // sends to this and upwards
}

void BaseWidget::OnSize(wxSizeEvent& event)
{
    if (!IsInitialized())
        return;

    // Calculate available width
    const int newWidth = m_parent->GetClientSize().GetWidth();

    if (!(newWidth == m_widgetPanel->GetMaxWidth() && newWidth == m_widgetPanel->GetMinWidth()))
    {
        // Constrain child widgets
        const wxSize newSize(newWidth, -1);
        m_widgetPanel->SetMinSize(newSize);
        m_widgetPanel->SetMaxSize(newSize);
    }

    event.Skip();
}

void BaseWidget::OnVetoMouseWheel(wxMouseEvent& event)
{
    wxMouseEvent evt(event); // copy event
    m_parent->GetEventHandler()->ProcessEvent(evt);
}

///////////////////////////////////////////////////////////

void BaseWidget::DoSetSelected()
{
    m_widgetPanel->SetBackgroundColour(m_currColour);
    m_widgetPanel->Refresh();
}

void BaseWidget::DoSetDescription()
{
    PersistentToolTip::SetToolTip(m_widgetPanel, m_description);

    PersistentToolTip::SetToolTip(m_derivedHeader, m_description);

    PersistentToolTip::SetToolTip(m_metadataPanel, m_description);

    SetDescriptionImpl();
}

void BaseWidget::DoSetColour()
{
    m_widgetPanel->SetBackgroundColour(m_normalColour);
    m_widgetPanel->Refresh();
}

void BaseWidget::DoUpdateMetadata(const wxString& key)
{
    auto it = m_currMetadata.find(key);
    if (it == m_currMetadata.end())
        return;

    const wxString label = it->second;

    auto widgetIt = m_currMetadataWidgets.find(key);
    if (widgetIt == m_currMetadataWidgets.end())
    {
        wxStaticText* staticText = new wxStaticText(m_metadataPanel, wxID_ANY, label);

        m_metaSizer->Add(staticText, 0, wxBOTTOM, 2);

        PersistentToolTip::SetToolTip(staticText, label);
        
        BindSelectable(staticText);

        m_currMetadataWidgets[key] = staticText;
    }
    else
    {
        wxStaticText* staticText = widgetIt->second;

        staticText->SetLabel(label);
    
        PersistentToolTip::SetToolTip(staticText, label);
    }

}

void BaseWidget::DoUpdateMetadataBulk()
{
    bool layoutDirty = false;

    // Remove widgets
    for (auto it = m_currMetadataWidgets.begin(); it != m_currMetadataWidgets.end(); )
    {
        if (m_currMetadata.find(it->first) == m_currMetadata.end())
        {
            wxStaticText* staticText = it->second;

            PersistentToolTip::RemoveToolTip(staticText);
            UnbindSelectable(staticText);
            m_metaSizer->Detach(staticText);
            staticText->Destroy();
            it = m_currMetadataWidgets.erase(it);
            layoutDirty = true;
        }
        else
        {
            ++it;
        }
    }

    // Add/update widgets
    for (const auto& [key, value] : m_currMetadata)
    {
        const wxString newLabel = key + ": " + value;

        auto it = m_currMetadataWidgets.find(key);
        if (it == m_currMetadataWidgets.end())
        {
            wxStaticText* staticText = new wxStaticText(m_metadataPanel, wxID_ANY, newLabel);
            m_metaSizer->Add(staticText, 0, wxBOTTOM, 2);

            PersistentToolTip::SetToolTip(staticText, newLabel);
            BindSelectable(staticText);

            m_currMetadataWidgets[key] = staticText;
            layoutDirty = true;
        }
        else if (it->second->GetLabel() != newLabel)
        {
            wxStaticText* staticText = it->second;

            staticText->SetLabel(newLabel);

            PersistentToolTip::SetToolTip(staticText, newLabel);

            layoutDirty = true;
        }

    }

    if (layoutDirty)
    {
        m_metadataPanel->Layout();

        wxCommandEvent evt(GUI_TOGGLE_META_WIDGET, m_widgetPanel->GetId());
        evt.SetInt(m_metadataPanel->IsShown());

        ProcessEvent(evt);
    }
}