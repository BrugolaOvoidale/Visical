#include "CameraWidgetList.hpp"
#include <camera/ICameraInfo.hpp>
#include "CameraWidgetEvents.hpp"


CameraWidgetList::CameraWidgetList(
	wxWindow* parent,
	wxWindowID winid,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxString& name)
	: 
    BaseWidgetList(parent,
        winid,
        pos,
        size,
        style,
        name)
{
}

//////////////////////////////////////////////////////////////////////////

bool CameraWidgetList::AppendCamera(
    const std::shared_ptr<ICameraInfo>& camera,
    const wxArrayString& camBehaviours)
{
    const std::string& serialNmb = camera->serialNumber();

    auto it = m_cameraIdtoItem.find(serialNmb);
    if (it != m_cameraIdtoItem.end())
        return false;

    std::shared_ptr<CameraWidget> cameraWidget = std::make_shared<CameraWidget>(this, camera, camBehaviours);

    m_cameraIdtoItem[serialNmb] = cameraWidget;
    m_itemToCameraId[cameraWidget] = serialNmb;

    // Bind custom event
    cameraWidget->Bind(GUI_SELECT_CAMERA, &CameraWidgetList::OnWidgetClicked, this);

    BaseWidgetList::AppendWidget(cameraWidget);

    return true;
}

std::shared_ptr<CameraWidget> CameraWidgetList::GetWidget(const wxString& cameraId)
{
    auto it = m_cameraIdtoItem.find(cameraId);
    if (it == m_cameraIdtoItem.end())
        return nullptr;

    return it->second;
}

void CameraWidgetList::RemoveCamera(const wxString& toRemoveId)
{
    auto it = m_cameraIdtoItem.find(toRemoveId);
    if (it == m_cameraIdtoItem.end())
        return;

    std::shared_ptr<CameraWidget> camera = it->second;

    m_cameraIdtoItem.erase(toRemoveId);

    m_itemToCameraId.erase(camera);

    BaseWidgetList::RemoveWidget(camera);
}

bool CameraWidgetList::SelectCamera(const wxString& cameraId)
{
    auto it = m_cameraIdtoItem.find(cameraId);
    if (it == m_cameraIdtoItem.end())
        return false;

    it->second->SetSelected(true);
    m_selectedWidget = it->second;

    return true;
}

std::optional<wxString> CameraWidgetList::GetSelectedCamera()
{
    if (!m_selectedWidget)
        return std::nullopt;

    return m_itemToCameraId.at(std::static_pointer_cast<CameraWidget>(m_selectedWidget));
}

//////////////////////////////////////////////////////////////////////////

void CameraWidgetList::RemoveSelectedWidgetImpl(const std::shared_ptr<BaseWidget>& baseWidget)
{
    std::shared_ptr<CameraWidget> toRemoveWidget = std::static_pointer_cast<CameraWidget>(baseWidget);

    const wxString toRemoveId = m_itemToCameraId.at(toRemoveWidget);

    m_itemToCameraId.erase(toRemoveWidget);

    m_cameraIdtoItem.erase(toRemoveId);
}

void CameraWidgetList::RemoveAllWidgetsImpl()
{
    m_cameraIdtoItem.clear();
    m_itemToCameraId.clear();
}

