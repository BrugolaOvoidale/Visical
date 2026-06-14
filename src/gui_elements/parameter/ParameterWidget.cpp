#include "ParameterWidget.hpp"
#include <wx/timer.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/generic/statbmpg.h>
#include <wx/stattext.h>
#include <wx/statline.h>
#include <wx/bmpbuttn.h>
#include <UtilityFunctions.hpp>
#include <parameter/ParameterInfo.hpp>
#include <gui_elements/persistent_tooltip/PersistentToolTip.hpp>
#include "ParameterWidgetEvents.hpp"


///////////////////////////////////////////////

static const wxString ICON_INFO{ "resources/information.png" };
static const wxString ICON_RESET{ "resources/reset.png" };

///////////////////////////////////////////////

ParameterWidget::ParameterWidget(
    wxWindow* parent,
    const std::shared_ptr<ParameterInfo>& param,
    bool hasResetButton)
    : BaseWidget(parent, false),
    m_parameterId(param->name()),
    m_categoryId(param->category()),
    m_valueType(param->dataType()),
    m_hasResetButton(hasResetButton),
    m_debounceTimer(this)
{
    // Bind
    Bind(wxEVT_TIMER, &ParameterWidget::OnDebounceTimer, this);

    SetAccessMode(param->accessMode());

    SetImposedAccessMode(
        param->imposedAccessMode(),
        param->imposedAccessModeReason().value_or("No reason provided")
    );

    SetParameterName(param->displayName().value_or(param->name()));

    SetUnit(param->unit().value_or(""));

    SetDescription(param->description().value_or(""));
}

//////////////////////////////////////////////////////////////////////////

void ParameterWidget::Update(const std::shared_ptr<ParameterInfo>& param)
{
    if (m_valueType != param->dataType() ||
        m_parameterId != param->name() ||
        m_categoryId != param->category())
    {
        return;
    }

    SetAccessMode(param->accessMode());

    SetImposedAccessMode(
        param->imposedAccessMode(),
        param->imposedAccessModeReason().value_or("No reason provided")
    );

    SetParameterName(param->displayName().value_or(param->name()));

    SetUnit(param->unit().value_or(""));

    SetDescription(param->description().value_or(""));

	UpdateImpl(param);
}

void ParameterWidget::SetAccessMode(AccessMode accessMode)
{
    if (m_accessMode == accessMode)
        return;

    m_accessMode = accessMode;

    if (IsInitialized())
        DoSetAccessMode();
}

void ParameterWidget::SetImposedAccessMode(
    AccessMode imposedAccessMode,
    const wxString& reason)
{
    m_imposedAccessMode = imposedAccessMode;
    m_isWritable = (imposedAccessMode == AccessMode::READWRITE || imposedAccessMode == AccessMode::WRITEONLY) ? true : false;

    if (m_accessMode != m_imposedAccessMode)
        m_imposedAccessModeReason = reason;
    else
        m_imposedAccessModeReason.reset();

    if (IsInitialized())
        DoSetImposedAccessMode();
}

DataType ParameterWidget::GetParamType() const
{
    return m_valueType;
}

const wxString& ParameterWidget::GetParameterId() const
{
    return m_parameterId;
}

const wxString& ParameterWidget::GetCategoryId() const
{
    return m_categoryId;
}

void ParameterWidget::SetParameterName(const wxString& parameterName)
{
    if (m_paramName == parameterName)
        return;

    m_paramName = parameterName;

    if (IsInitialized())
        DoSetParameterName();
}

const wxString& ParameterWidget::GetParameterName() const
{
    return m_paramName;
}

const wxString& ParameterWidget::GetUnit() const
{
    return m_unit;
}

bool ParameterWidget::IsWritable() const
{
    return m_isWritable;
}

void ParameterWidget::SetUnit(const wxString& unit)
{
    if (m_unit == unit)
        return;

    m_unit = unit;

    if (IsInitialized())
        DoSetUnit();
}

void ParameterWidget::MarkAsDirty(bool dirty)
{
    if (m_isDirty == dirty)
        return;

    m_isDirty = dirty;

    if (IsInitialized())
        DoMarkAsDirty();
}

bool ParameterWidget::IsDirty() const
{
    return m_isDirty;
}

//////////////////////////////////////////////////////////////////////////

wxPanel* ParameterWidget::CreateHeader()
{
    // Header row
    m_parameterHeader = new wxPanel(m_widgetPanel);

    wxBoxSizer* headerSizer = new wxBoxSizer(wxHORIZONTAL);

    wxImage infoImage(ICON_INFO, wxBITMAP_TYPE_PNG);
    infoImage.Rescale(18, 18);

    m_imposedAccessModeBmp = new wxGenericStaticBitmap(m_parameterHeader, wxID_ANY, infoImage);
    BindSelectable(m_imposedAccessModeBmp);

    headerSizer->Add(m_imposedAccessModeBmp, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 10);
    m_imposedAccessModeBmp->Hide();

    // Parameter name
    m_parameterNameTxt = new wxStaticText(
        m_parameterHeader,
        wxID_ANY,
        wxEmptyString,
        wxDefaultPosition,
        wxSize(160, -1), // fixed column width
        wxST_ELLIPSIZE_END
    );

    BindSelectable(m_parameterNameTxt);

    headerSizer->Add(m_parameterNameTxt, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);

    // Separator
    wxStaticLine* sep1 = new wxStaticLine(
        m_parameterHeader, wxID_ANY,
        wxDefaultPosition,
        wxSize(1, 20),
        wxLI_VERTICAL);

    headerSizer->Add(sep1, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);

    // Value control area
    m_valueControlSizer = new wxBoxSizer(wxHORIZONTAL);

    m_valueControl = CreateValueControl();

    m_valueControl->Bind(wxEVT_MOUSEWHEEL, &ParameterWidget::OnVetoMouseWheel, this);
    BindSelectable(m_valueControl, true);

    m_valueControlSizer->Add(m_valueControl, 0);

    m_unitTxt = new wxStaticText(m_parameterHeader, wxID_ANY, wxEmptyString);

    BindSelectable(m_unitTxt);

    m_valueControlSizer->Add(m_unitTxt, 0);

    headerSizer->Add(m_valueControlSizer, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);

    // Reset button
    if (m_hasResetButton)
    {
        // Spacer pushes reset button to right
        headerSizer->AddStretchSpacer();

        wxImage resetImage(ICON_RESET, wxBITMAP_TYPE_PNG);
        resetImage.Rescale(18, 18);

        m_resetButton = new wxBitmapButton(
            m_parameterHeader,
            wxID_ANY,
            resetImage
        );
        m_resetButton->Bind(wxEVT_BUTTON, &ParameterWidget::OnResetParameter, this);
        BindSelectable(m_resetButton, true);
        PersistentToolTip::SetToolTip(m_resetButton, "Reset parameter to its default");

        headerSizer->Add(m_resetButton, 0, wxALIGN_CENTER_VERTICAL);
    }

    m_parameterHeader->SetSizer(headerSizer);

    if (!IsWritable())
        ReadOnlyModeImpl();


    m_widgetPanel->Layout();

    return m_parameterHeader;
}

void ParameterWidget::StoreLatestSetParamEvent(const ParameterChangedEvent& event, int milliseconds)
{
    m_lastSetParamEvent = new ParameterChangedEvent(event);

    if (milliseconds <= 0)
    {
        ProcessLatestSetParamEvent();
    }
    else
    {
        m_debounceTimer.StartOnce(milliseconds);
    }
}

void ParameterWidget::DoSetAccessMode()
{
    if (m_isWritable)
        RWMode();
    else
        ReadOnlyMode();
}

void ParameterWidget::SetDescriptionImpl()
{
    PersistentToolTip::SetToolTip(m_parameterNameTxt, m_paramName + ":\n" + m_description);

    PersistentToolTip::SetToolTip(m_unitTxt, m_description);
}

void ParameterWidget::OnSpecializedWidgetClick(wxMouseEvent& event)
{
    // Let the parent know this parameter was clicked
    ParameterEvent evt(GUI_SELECT_PARAM, m_widgetPanel->GetId());
    evt.SetClientData(wxUIntToPtr(GetWidgetId()));
    evt.SetParameterId(GetParameterId());
    evt.SetCategoryId(GetCategoryId());

    // Send it to parent
    ProcessEvent(evt);         // sends to this and upwards

    event.Skip();
}

//////////////////////////////////////////////////////////////////////////

void ParameterWidget::InitializeImpl()
{
    DoSetParameterName();

    DoSetUnit();

    DoSetAccessMode();

    DoSetImposedAccessMode();

    DoMarkAsDirty();

    InitializeParameter();
}

void ParameterWidget::DoSetParameterName()
{
    m_parameterNameTxt->SetLabel(m_paramName);
}

void ParameterWidget::DoSetUnit()
{
    m_unitTxt->SetLabel(m_unit);
}

void ParameterWidget::DoSetImposedAccessMode()
{
    if (m_accessMode != m_imposedAccessMode)
    {
        m_imposedAccessModeBmp->Show();

        PersistentToolTip::SetToolTip(
            m_imposedAccessModeBmp,
            "Actual access mode: " + UtilityFunctions::stringFromEnum(m_accessMode).value()
            + "\nImposed access mode: " + UtilityFunctions::stringFromEnum(m_imposedAccessMode).value()
            + "\nReason: " + m_imposedAccessModeReason.value()
        );
    }
    else
    {
        m_imposedAccessModeBmp->Hide();

        m_imposedAccessModeReason.reset();

        PersistentToolTip::RemoveToolTip(m_imposedAccessModeBmp);
    }
}

void ParameterWidget::DoMarkAsDirty()
{
    if (m_isDirty)
    {
        m_parameterNameTxt->SetFont(
            m_parameterNameTxt->GetFont().MakeBold()
        );
    }
    else
    {
        wxFont aFont = m_parameterNameTxt->GetFont();
        aFont.SetWeight(wxFONTWEIGHT_NORMAL);
        m_parameterNameTxt->SetFont(
            aFont
        );
    }

    m_parameterHeader->Layout();
}

void ParameterWidget::RWMode()
{
    if (m_hasResetButton)
    {
        m_resetButton->Enable();
        m_resetButton->Show();
    }

    RWModeImpl();
}

void ParameterWidget::ReadOnlyMode()
{
    if (m_hasResetButton)
    {
        m_resetButton->Disable();
        m_resetButton->Hide();
    }

    ReadOnlyModeImpl();
}

void ParameterWidget::ProcessLatestSetParamEvent()
{
    if (m_lastSetParamEvent)
    {
        // Send it to parent
        ProcessEvent(*m_lastSetParamEvent);         // sends to this and upwards

        // Consumed
        m_lastSetParamEvent = nullptr;
    }
}

void ParameterWidget::OnResetParameter(wxCommandEvent& event)
{
    // Let the parent know this parameter requested reset
    ParameterEvent evt(GUI_RESET_PARAM, m_widgetPanel->GetId());
    evt.SetParameterId(GetParameterId());
    evt.SetCategoryId(GetCategoryId());

    // Send it to parent
    ProcessEvent(evt);         // sends to this and upwards

    event.Skip();
}

void ParameterWidget::OnDebounceTimer(wxTimerEvent&)
{
    ProcessLatestSetParamEvent();
}