#pragma once
#include <unordered_set>
#include <wx/event.h>
#include <wx/colour.h>


///////////////////////////////////////////////

// Payload: int -> toggled [0, 1]
wxDECLARE_EVENT(GUI_TOGGLE_META_WIDGET, wxCommandEvent);

///////////////////////////////////////////////


// Forward declarations
class wxPanel;
class wxSizer;
class wxBoxSizer;
class wxStaticBoxSizer;
class wxButton;
class wxStaticText;


/**
 * @brief Abstract base class for GUI components with an optional metadata drawer.
 *
 * It is designed to be UI virtualized via 'Initialize()' and 'Deinitialize()', living inside a
 * 'BaseWidgetList', which manages the lifecycle of its child widgets and optimizes performance
 * by only keeping a subset of widgets initialized at any time.
 */
class BaseWidget : public wxEvtHandler
{
public:
    //-----------------------------------------------------------------------------
    // Constants
    //-----------------------------------------------------------------------------

    static inline const wxColour DEFAULT_GRAY{ 240, 240, 240 };
    static inline const wxColour HIGHLIGHT_BLUE{ 200, 220, 255 };
    static inline const wxColour HIGHLIGHT_YELLOW{ 255, 250, 170 };
    static inline const wxColour HIGHLIGHT_GREEN{ 180, 255, 180 };
    static inline const wxColour HIGHLIGHT_ORANGE{ 255, 200, 130 };
    static inline const wxColour HIGHLIGHT_RED{ 255, 128, 128 };


    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    ~BaseWidget();


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /** @brief Returns the unique runtime ID assigned to this widget instance. */
    wxUIntPtr GetWidgetId() const;

    /**
     * @brief Orchestrates the creation of the UI.
     * @param startShowed If true, the widget is visible immediately.
     */
    void Initialize(bool startShowed = true);

    /** @brief Cleans up UI resources and bindings. */
    void Deinitialize();

    /** @brief Returns the main panel containing the entire widget UI. */
    wxPanel* GetWidgetPanel() const;

    /** @brief Returns the suggested height for layout calculations. */
    int GetPreferredHeight() const;

    /** @brief Returns true if Initialize() has been successfully called. */
    bool IsInitialized() const;

    /** @brief Returns true if the metadata panel was enabled during construction. */
    bool HasMetadataPanel() const;

    /** @brief Sets the visual selection state and updates the background color. */
    void SetSelected(bool selected);

    /** @brief Returns the current selection status. */
    bool IsSelected() const;

    /** @brief Updates the text description of the widget. */
    void SetDescription(const wxString& description);

    /** @brief Returns the current description text. */
    const wxString& GetDescription() const;

    /** @brief Changes the current background color. */
    void SetColour(const wxColour& colour);

    /** @brief Returns the current background color. */
    wxColour GetColour() const;

    /**
     * @brief Updates or adds a single key-value pair in the metadata panel.
     * @param key The identifier for the metadata entry.
     * @param value The text to display.
     */
    void UpdateMetadata(const wxString& key, const wxString& value);

    /** @brief Bulk updates metadata from a map. */
    void UpdateMetadata(const std::unordered_map<wxString, wxString>& metadata);

protected:
    /**
     * @brief Constructs the base widget components.
     * @param parent The parent wxWindow.
     * @param hasMetaPanel If true, creates a collapsible section for metadata.
     */
    BaseWidget(
        wxWindow* parent,
        bool hasMetaPanel = true
    );

    // Derived classes build header here
    virtual wxPanel* CreateHeader() = 0;

    // Implementation hook to handle description changes in derived UI.
    virtual void SetDescriptionImpl() = 0;

    // Implementation hook for custom logic during initialization
    virtual void InitializeImpl() = 0;

    // Adds a window to the metadata layout.
    void AddMetadataItem(wxWindow* item);

    // Adds a sizer to the metadata layout.
    void AddMetadataItem(wxSizer* item);

    /**
     * @brief Binds mouse events of a child window to the selection logic of this widget.
     * @param w The child window to bind.
     * @param keepSelectedOnClick If true, clicking this element won't toggle selection off.
     */
    void BindSelectable(
        wxWindow* w,
        bool keepSelectedOnClick = false
    );

    // Removes selection bindings from a child window.
    void UnbindSelectable(wxWindow* w);

    // Returns the base color when the widget is NOT highlighted.
    wxColour GetNormalColour() const;


    //-----------------------------------------------------------------------------
    // Events
    //-----------------------------------------------------------------------------

    void OnWidgetClick(wxMouseEvent& event);

    virtual void OnSpecializedWidgetClick(wxMouseEvent& event) = 0;

    void OnToggleMetaPanel(wxCommandEvent&);

    void OnSize(wxSizeEvent& event);

    void OnVetoMouseWheel(wxMouseEvent& event);

private:
    void DoSetSelected();

    void DoSetDescription();

    void DoSetColour();

    void DoUpdateMetadata(const wxString& key);

    void DoUpdateMetadataBulk();

protected:
    // Parent window reference.
	wxWindow* m_parent;

    // The root panel of this widget.
    wxPanel* m_widgetPanel;

    // The collapsible metadata container.
    wxPanel* m_metadataPanel;

    // Internal description string.
    wxString m_description{ wxEmptyString };

private:
    // Unique instance ID.
    const wxUIntPtr m_widgetId;

    // Metadata capability flag.
    const bool m_hasMetadataPanel;

    // Panel created by CreateHeader().
    wxPanel* m_derivedHeader;

    // Button to show/hide metadata.
    wxButton* m_toggleBtn;

    // Flag if Initialize() has been successfully called
    bool m_initialized{ false };

    // Sizer for the header area.
    wxBoxSizer* m_headerSizer;

    // Main vertical sizer for the widget.
    wxBoxSizer* m_rootSizer;

    // Container for individual metadata rows.
    wxBoxSizer* m_metaItemsSizer;

    // Styled sizer for the metadata panel.
    wxStaticBoxSizer* m_metaSizer;

    // Current active background color.
    wxColour m_currColour{ DEFAULT_GRAY };

    wxColour m_normalColour{ DEFAULT_GRAY };

    std::unordered_map<wxString, wxString> m_currMetadata;

    std::unordered_map<wxString, wxStaticText*> m_currMetadataWidgets;

    std::unordered_set<wxObject*> m_bindedSelectElements;

    std::unordered_set<wxObject*> m_keepSelectedOnClick;

    // Flag if the widgte is selected
    bool m_isSelected{ false };

    static inline wxUIntPtr m_widgetIdCounter{ 1 };
};
