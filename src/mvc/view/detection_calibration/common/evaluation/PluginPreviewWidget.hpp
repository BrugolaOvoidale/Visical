#pragma once
#include <memory>
#include <wx/bmpbuttn.h>
#include <gui_elements/base_widget/BaseWidget.hpp>


// Forward declarations
class PluginContextInfo;
class wxBitmapButton;


/**
 * @brief A graphic component handling the display for an individual evaluation plugin preview.
 */
class PluginPreviewWidget : public BaseWidget
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    /**
     * @brief Constructs a new Plugin Preview Widget.
     * @param parent Pointer to the parent window layer.
     * @param camera Shared handle to structural info describing this specific plugin instance.
     */
    PluginPreviewWidget(
        wxWindow* parent,
        const std::shared_ptr<PluginContextInfo>& pluginCtx
    );

    ~PluginPreviewWidget() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /** @brief Gets the unique string Id of the underlying plugin. */
    wxString GetPluginId() const;

    /**
     * @brief Completely refreshes the widget's internal data using a new plugin context.
     * @param pluginCtx The updated plugin context info layout.
     */
    void Update(const std::shared_ptr<PluginContextInfo>& pluginCtx);

    void EnablePlugin(bool enable = true);

    void DisablePlugin() { EnablePlugin(false); }

    /** @brief Formats and updates the text note/label associated with the last run. */
    void SetNote(const wxString& newNote);

    /** @brief Retrieves the currently displayed evaluation status note string. */
    const wxString& GetNote() const;

private:
    wxPanel* CreateHeader() override;

    void InitializeImpl() override;

    void SetDescriptionImpl() override;

    void DoSetNote();

    void DoEnable();


    //-----------------------------------------------------------------------------
    // Events
    //-----------------------------------------------------------------------------

    void OnSpecializedWidgetClick(wxMouseEvent& event) override;

    void OnAddRemovePlugin(wxCommandEvent& event);

protected:
    wxString m_pluginId;

    wxString m_note;

    bool m_isEnabled{ true };

    wxStaticText* m_noteItem;

    wxBitmapButton* m_addBtn;

    wxBitmap m_addBitmap;

    wxBitmap m_removeBitmap;
};
