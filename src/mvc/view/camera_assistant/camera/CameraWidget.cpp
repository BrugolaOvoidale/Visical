#include "CameraWidget.hpp"
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/animate.h>
#include <wx/statline.h>
#include <wx/stattext.h>
#include <wx/bmpbuttn.h>
#include <wx/choice.h>
#include <UtilityFunctions.hpp>
#include <camera/ICamera.hpp>
#include "CameraWidgetEvents.hpp"


static const wxString ICON_UNKNOWN{ "resources/gray_dot.gif" };
static const wxString ICON_LOADING{ "resources/progress.gif" };
static const wxString ICON_NOT_DETECTED{ "resources/check_failed.gif" };
static const wxString ICON_DETECTED{ "resources/check_success.gif" };
static const wxString ICON_IDLE{ "resources/success.gif" };
static const wxString ICON_GRABBING{ "resources/recording.gif" };
static const wxString ICON_ERROR{ "resources/warning.gif" };
static const wxString ICON_CHECK{ "resources/check.png" };
static const wxString ICON_CONNECT{ "resources/connect.png" };
static const wxString ICON_DISCONNECT{ "resources/disconnect.png" };

static const wxString CHECKING_TAG{ "Checking..." };
static const wxString UNKNOWN_TAG{ "UNKNOWN" };
static const wxString READY_TAG{ "Ready for connection" };
static const wxString NOT_DETECTED_TAG{ "NOT DETECTED" };
static const wxString CONNECTING_TAG{ "Connecting..." };
static const wxString CONNECTED_TAG{ "CONNECTED" };
static const wxString CONNECTION_FAILED_TAG{ "CONNECTION FAILED" };
static const wxString DISCONNECTING_TAG{ "Disconnecting..." };
static const wxString DISCONNECTION_FAILED_TAG{ "DISCONNECTION FAILED" };
static const wxString LIVE_ACQ_TAG{ "LIVE" };

static const wxString METADATA_INTERFACE{ "Interface type" };


/////////////////////////////////////////////////////

CameraWidget::CameraWidget(
    wxWindow* parent,
    const std::shared_ptr<ICameraInfo>& camera,
    const wxArrayString& camBehaviours)
    : BaseWidget(parent, true),
    m_cameraId(camera->serialNumber()),
    m_camBehaviours(camBehaviours)
{
    m_connectBitmap = wxImage(ICON_CONNECT, wxBITMAP_TYPE_PNG).Rescale(18, 18);

    m_disconnectBitmap = wxImage(ICON_DISCONNECT, wxBITMAP_TYPE_PNG).Rescale(18, 18);

    Update(camera);
}

//////////////////////////////////////////////////////////////////////////

void CameraWidget::Update(const std::shared_ptr<ICameraInfo>& camera)
{
    if (m_cameraId != camera->serialNumber())
        return;

    SetCameraState(camera->state());

    SetInterfaceType(
        UtilityFunctions::stringFromEnum<CameraInterface>(camera->interfaceType()).value()
    );
}

const wxString& CameraWidget::GetCameraId() const
{
    return m_cameraId;
}

void CameraWidget::SetCameraState(CameraState camState)
{
    if (camState == m_camState)
        return;
    
    m_camState = camState;

    if (IsInitialized() && m_camOp == Operation::NONE)
        DoSetState();
}

CameraState CameraWidget::GetCameraState() const
{
    return m_camState;
}

void CameraWidget::SetCameraOperation(Operation op)
{
    if (op == m_camOp)
        return;

    m_camOp = op;

    if (IsInitialized())
    {
		if (m_camOp != Operation::NONE)
        {
            DoSetOperation();
        }
        else
        {
			DoSetState();
        }
    }
}

CameraWidget::Operation CameraWidget::GetCameraOperation() const
{
    return m_camOp;
}

void CameraWidget::SetBehaviours(const wxArrayString& newBehaviours)
{
    if (m_camBehaviours == newBehaviours)
        return;

    m_camBehaviours = newBehaviours;

    if (IsInitialized())
        DoSetBehaviours();
}

bool CameraWidget::SelectBehaviour(const wxString& selBehaviour)
{
    if (m_currBehaviour == selBehaviour)
        return true;

    m_currBehaviour = selBehaviour;

    if (IsInitialized())
        return DoSelectBehaviour();

	return true;
}

const wxString& CameraWidget::GetSelectedBehaviour() const
{
    return m_interfaceTypeStr;
}

void CameraWidget::SetInterfaceType(const wxString& interfaceType)
{
    if (m_interfaceTypeStr == interfaceType)
        return;

	m_interfaceTypeStr = interfaceType;

    if (IsInitialized())
        DoSetInterfaceType();
}

const wxString& CameraWidget::GetInterfaceType() const
{
	return m_interfaceTypeStr;
}

//////////////////////////////////////////////////////////////////////////

wxPanel* CameraWidget::CreateHeader()
{
    // --- Header row ---
    wxPanel* header = new wxPanel(m_widgetPanel);

    wxBoxSizer* headerSizer = new wxBoxSizer(wxHORIZONTAL);

    // Status animation
    m_status = new wxAnimationCtrl(header, wxID_ANY);
    headerSizer->Add(m_status, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 12);
    BindSelectable(m_status);

    // Separator
    wxStaticLine* sep1 = new wxStaticLine(
        header, wxID_ANY,
        wxDefaultPosition,
        wxSize(1, 24),
        wxLI_VERTICAL);
    BindSelectable(sep1);

    headerSizer->Add(sep1, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 12);

    // Camera Id text
    m_cameraIdTxt = new wxStaticText(header, wxID_ANY, m_cameraId);
    m_cameraIdTxt->SetMinSize(wxSize(120, -1));
    BindSelectable(m_cameraIdTxt);

    headerSizer->Add(m_cameraIdTxt, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 12);

    // Separator
    wxStaticLine* sep2 = new wxStaticLine(
        header, wxID_ANY,
        wxDefaultPosition,
        wxSize(1, 24),
        wxLI_VERTICAL);
    BindSelectable(sep2);

    headerSizer->Add(sep2, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 12);

    // Status text
    m_statusTxt = new wxStaticText(header, wxID_ANY, wxEmptyString);
    BindSelectable(m_statusTxt);
    headerSizer->Add(m_statusTxt, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 12);

    // Spacer pushing buttons to the right
    headerSizer->AddStretchSpacer();


    // Check button
    wxBitmap checkBitmap = wxImage(ICON_CHECK, wxBITMAP_TYPE_PNG).Rescale(18, 18);

    m_checkButton = new wxBitmapButton(
        header,
        wxID_ANY,
        checkBitmap
    );
    m_checkButton->SetMinSize(m_widgetPanel->FromDIP(m_checkButton->GetBestSize()));
    m_checkButton->Bind(wxEVT_BUTTON, &CameraWidget::OnCheckCamera, this);
    BindSelectable(m_checkButton, true);

    headerSizer->Add(m_checkButton, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 6);

    // Connect button
    m_connectButton = new wxBitmapButton(
        header,
        DelaySelect::ConnectBtn,
        m_connectBitmap
    );
    m_connectButton->SetMinSize(m_widgetPanel->FromDIP(m_connectButton->GetBestSize()));
    m_connectButton->Bind(wxEVT_BUTTON, &CameraWidget::OnConnectDisconnectCamera, this);
    BindSelectable(m_connectButton, true);

    headerSizer->Add(m_connectButton, 0, wxALIGN_CENTER_VERTICAL);


    //
    wxBoxSizer* behaviourSizer = new wxBoxSizer(wxHORIZONTAL);

    wxStaticText* behaviourTxt = new wxStaticText(m_metadataPanel, wxID_ANY, "Discovery behaviour");
    BindSelectable(behaviourTxt);

    m_behaviourChoice = new wxChoice(m_metadataPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_camBehaviours);
    BindSelectable(behaviourTxt, true);
    m_behaviourChoice->Bind(wxEVT_CHOICE, &CameraWidget::OnChangeBehaviour, this);


    behaviourSizer->Add(behaviourTxt, 0, wxLEFT, 0);
    behaviourSizer->Add(m_behaviourChoice, 0, wxLEFT, 0);

    AddMetadataItem(behaviourSizer);

    //
    m_interfaceType = new wxStaticText(m_metadataPanel, wxID_ANY, wxEmptyString);

    AddMetadataItem(m_interfaceType);

    header->SetSizer(headerSizer);

    return header;
}

void CameraWidget::InitializeImpl()
{
    DoSetBehaviours();

    DoSelectBehaviour();

    DoSetInterfaceType();

    DoSetState();
}

void CameraWidget::SetDescriptionImpl()
{
}

void CameraWidget::DoSetState()
{
    switch (m_camState.activity())
    {
    case CameraState::Activity::GRABBING:
        GrabbingState();
        break;
    case CameraState::Activity::IDLE:
        IdleState();
        break;
    case CameraState::Activity::UNKNOWN:
        switch (m_camState.connection())
        {
        case CameraState::Connection::CONNECTED:
            IdleState();
            break;
        case CameraState::Connection::DISCONNECTED:
            DetectedState();
            break;
        case CameraState::Connection::UNKNOWN:
            switch (m_camState.presence())
            {
            case CameraState::Presence::DETECTED:
                DetectedState();
                break;
            case CameraState::Presence::NOT_DETECTED:
                NotDetectedState();
                break;
            case CameraState::Presence::UNKNOWN:
                UnknownState();
                break;
            }
            break;
        }
        break;

    default:
        UnknownState();
        break;
    }
}

void CameraWidget::DoSetOperation()
{
    switch (m_camOp)
    {
        case Operation::CHECKING:
            BusyState(CHECKING_TAG);

            break;

        case Operation::CONNECTING:
            BusyState(CONNECTING_TAG);

            break;

        case Operation::DISCONNECTING:
            BusyState(DISCONNECTING_TAG);

            break;

        default:
            break;
    }
}

void CameraWidget::DoSetBehaviours()
{
    m_behaviourChoice->Set(m_camBehaviours);
}

bool CameraWidget::DoSelectBehaviour()
{
    return m_behaviourChoice->SetStringSelection(m_currBehaviour);
}

void CameraWidget::DoSetInterfaceType()
{
    m_interfaceType->SetLabel(m_interfaceTypeStr);
}

void CameraWidget::UnknownState()
{
    if (m_status->LoadFile(ICON_UNKNOWN))
    {
        m_status->Stop(); // Stop the animation
    }

    m_connectButton->SetBitmap(m_connectBitmap);
    m_isDisconnectBitmap = false;

    m_checkButton->Enable();
    m_connectButton->Disable();
}

void CameraWidget::BusyState(const wxString& statusTxtTag)
{
    if (m_status->LoadFile(ICON_LOADING))
    {
        m_status->Play(); // Start the animation
    }

    m_connectButton->SetBitmap(m_connectBitmap);
    m_isDisconnectBitmap = false;

    m_checkButton->Disable();
    m_connectButton->Disable();

    m_statusTxt->SetLabel(statusTxtTag);
}

void CameraWidget::NotDetectedState()
{
    if (m_status->LoadFile(ICON_NOT_DETECTED))
    {
        m_status->Stop(); // Stop the animation
    }

    m_connectButton->SetBitmap(m_connectBitmap);
    m_isDisconnectBitmap = false;

    m_checkButton->Enable();
    m_connectButton->Disable();

    m_statusTxt->SetLabel(NOT_DETECTED_TAG);
}

void CameraWidget::DetectedState()
{
    if (m_status->LoadFile(ICON_DETECTED))
    {
        m_status->Stop(); // Stop the animation
    }

    m_connectButton->SetBitmap(m_connectBitmap);
    m_isDisconnectBitmap = false;

    m_checkButton->Enable();
    m_connectButton->Enable();

    m_statusTxt->SetLabel(READY_TAG);
}

void CameraWidget::IdleState()
{
    if (m_status->LoadFile(ICON_IDLE))
    {
        m_status->Stop(); // Stop the animation
    }

    m_connectButton->SetBitmap(m_disconnectBitmap);
    m_isDisconnectBitmap = true;

    m_checkButton->Enable();
    m_connectButton->Enable();

    m_statusTxt->SetLabel(CONNECTED_TAG);
}

void CameraWidget::GrabbingState()
{
    if (m_status->LoadFile(ICON_GRABBING))
    {
        m_status->Play(); // Start the animation
    }

    m_connectButton->SetBitmap(m_disconnectBitmap);
    m_isDisconnectBitmap = true;

    m_checkButton->Disable();
    m_connectButton->Disable();

    m_statusTxt->SetLabel(LIVE_ACQ_TAG);
}

void CameraWidget::OnSpecializedWidgetClick(wxMouseEvent& event)
{
    if (event.GetId() != DelaySelect::ConnectBtn)
    {
        // Let the parent know this camera was clicked
        wxCommandEvent evt(GUI_SELECT_CAMERA, m_widgetPanel->GetId());
        evt.SetClientData(wxUIntToPtr(GetWidgetId()));
        evt.SetString(GetCameraId());

        // Send it to parent
        ProcessEvent(evt);  // sends to this and upwards
    }

    event.Skip();
}

void CameraWidget::OnCheckCamera(wxCommandEvent& event)
{
    // Let the parent know this camera was clicked
    wxCommandEvent evt(GUI_CHECK_CAMERA, m_widgetPanel->GetId());
    evt.SetString(GetCameraId());

    // Send it to parent
    ProcessEvent(evt);         // sends to this and upwards

    event.Skip();
}

void CameraWidget::OnConnectDisconnectCamera(wxCommandEvent& event)
{
    // Let the parent know this camera was clicked
    wxCommandEvent evt;
    evt.SetString(GetCameraId());

    if (m_isDisconnectBitmap)
    {
        evt.SetEventType(GUI_DISCONNECT_CAMERA);
    }
    else
    {
        evt.SetEventType(GUI_CONNECT_CAMERA);
    }

    // Send it to parent
    ProcessEvent(evt);         // sends to this and upwards


    // Let the parent know this camera was clicked
    wxCommandEvent evtSelect(GUI_SELECT_CAMERA, m_widgetPanel->GetId());
    evt.SetClientData(wxUIntToPtr(GetWidgetId()));
    evtSelect.SetString(GetCameraId());

    // Send it to parent
    ProcessEvent(evtSelect);  // sends to this and upwards

    event.Skip();
}

void CameraWidget::OnChangeBehaviour(wxCommandEvent& event)
{
    m_currBehaviour = m_behaviourChoice->GetStringSelection();

    // Let the parent know this behaviour changed
    wxCommandEvent evt(GUI_CHANGE_BEHAVIOUR, m_widgetPanel->GetId());
    evt.SetString(GetSelectedBehaviour());

    // Send it to parent
    ProcessEvent(evt);         // sends to this and upwards

    event.Skip();
}