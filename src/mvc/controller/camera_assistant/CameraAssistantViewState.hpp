#pragma once
#include <wx/string.h>
#include <parameter/ParameterDefs.hpp>
#include "CameraAssistantDefs.hpp"


class CameraAssistantViewState
{
public:
    CameraAssistantViewState() = default;

public:
    void UnselectCamera() { m_selectedCamera = wxEmptyString; }
    void SetSelectedCamera(const wxString& cameraId) { m_selectedCamera = cameraId; }
    const wxString& GetSelectedCamera() const { return m_selectedCamera; }

    void UnselectCategory() { m_category = wxEmptyString; }
    void SetCategory(const wxString& category) { m_category = category; }
    const wxString& GetCategory() const { return m_category; }

    void UnselectVisibilityMode() { m_visibility = VisibilityLevel::UNDEFINED; }
    void SetVisibilityMode(VisibilityLevel visibility) { m_visibility = visibility; }
    VisibilityLevel GetVisibilityMode() const { return m_visibility; }

    void SetSelectedOpenBehaviour(const wxString& behaviour) { m_openBehaviour = behaviour; }
    const wxString& GetSelectedOpenBehaviour() const { return m_openBehaviour; }

    void SetSelectedDiscoveryBehaviour(const wxString& behaviour) { m_discoveryBehaviour = behaviour; }
    const wxString& GetSelectedDiscoveryBehaviour() const { return m_discoveryBehaviour; }

    void SetSelectedCameraSource(CameraSource src) { m_cameraSource = src; }
    CameraSource GetSelectedCameraSource() const { return m_cameraSource; }

private:
    wxString m_selectedCamera = wxEmptyString;

    bool m_disconnecting = false;

    wxString m_category = wxEmptyString;

    VisibilityLevel m_visibility = VisibilityLevel::UNDEFINED;

    wxString m_openBehaviour;

    wxString m_discoveryBehaviour;

    CameraSource m_cameraSource;
};