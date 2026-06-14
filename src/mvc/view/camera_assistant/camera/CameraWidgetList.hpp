#pragma once
#include <gui_elements/base_widget/BaseWidgetList.hpp>
#include "CameraWidget.hpp"


/**
 * @brief A scrollable UI container that manages and displays a virtual list of unique CameraWidgets.
 */
class CameraWidgetList : public BaseWidgetList
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    CameraWidgetList(
        wxWindow* parent,
        wxWindowID winid = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxVSCROLL | wxSIMPLE_BORDER,
        const wxString& name = wxASCII_STR(wxPanelNameStr)
    );

    ~CameraWidgetList() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /**
     * @brief Instantiates a new CameraWidget and appends it to the end of the list view.
     * @param camera Metadata configuration object used to initialize the camera stream widget.
     * @return false if camera is already in the list, false otherwise.
     */
    bool AppendCamera(
        const std::shared_ptr<ICameraInfo>& camera,
        const wxArrayString& camBehaviours
    );

    /**
    * @brief Retrieves a camera widget instance by its unique identifier string.
    * @param cameraId The unique string identifier of the target camera widget.
    * @return A shared pointer to the requested CameraWidget, or nullptr if not found.
    */
    std::shared_ptr<CameraWidget> GetWidget(const wxString& cameraId);

    /**
     * @brief Removes a camera widget from the UI list and destroys its tracking references.
     * @param toRemoveId The unique string identifier of the camera to remove.
     */
    void RemoveCamera(const wxString& toRemoveId);

    /**
     * @brief Programmatically changes the selection focus to a specific camera widget.
     * @param cameraId The unique string identifier of the camera to select.
     * @return true if the camera was found and successfully selected, false otherwise.
     */
    bool SelectCamera(const wxString& cameraId);

    /** * @brief Returns the selected camera ID, std::nullopt otherwise. */
    std::optional<wxString> GetSelectedCamera();

private:
    void RemoveSelectedWidgetImpl(const std::shared_ptr<BaseWidget>& baseWidget) override;

    void RemoveAllWidgetsImpl() override;

private:
    std::unordered_map<wxString, std::shared_ptr<CameraWidget>> m_cameraIdtoItem;

    std::unordered_map<std::shared_ptr<CameraWidget>, wxString> m_itemToCameraId;
};