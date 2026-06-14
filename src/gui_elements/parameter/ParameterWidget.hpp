#pragma once
#include <memory>
#include <wx/timer.h>
#include <gui_elements/base_widget/BaseWidget.hpp>
#include <parameter/ParameterDefs.hpp>


// Forward declarations
class ParameterInfo;
class ParameterChangedEvent;
class wxTimerEvent;
class wxBitmapButton;
class wxGenericStaticBitmap;


/**
 * @brief Abstract base class representing a visual control tied to a system parameter.
 *
 * This class provides a structural layout for custom UI parameter widgets
 * and handles common logic.
 */
class ParameterWidget : public BaseWidget
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    ~ParameterWidget() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /**
     * @brief Updates the widget's UI elements using data from a modified parameter object.
     * @param param Shared pointer to the source ParameterInfo dataset.
     */
    void Update(const std::shared_ptr<ParameterInfo>& param);

    /**
     * @brief Modifies the user operational permissions for this widget.
     * @param accessMode The target operational permissions setting.
     */
    void SetAccessMode(AccessMode accessMode);

    /**
     * @brief Overrides standard dynamic permissions with a hard forced runtime status constraint.
     * @param imposedAccessMode The forced baseline permission state.
     * @param reason Explanatory user-facing string detailing why permissions are constrained.
     */
    void SetImposedAccessMode(
        AccessMode imposedAccessMode,
        const wxString& reason
    );

    /** @brief Gets the structural data typing of the bounded parameter. */
    DataType GetParamType() const;

    /** @brief Retrieves the unique internal tracking string identification for the underlying parameter. */
    const wxString& GetParameterId() const;

    /** @brief Retrieves the visual organizational structural component classification category string. */
    const wxString& GetCategoryId() const;

    /** @brief Sets a new visible identification label string for the parameter text block. */
    void SetParameterName(const wxString& parameterName);

    /** @brief Gets the active visible description designation text assigned to the parameter label. */
    const wxString& GetParameterName() const;

    /** @brief Dynamically changes the measurement configuration suffix unit scalar string notation. */
    void SetUnit(const wxString& unit);

    /** @brief Returns the engineering text notation label representing unit tracking layout. */
    const wxString& GetUnit() const;

    /** @brief Checks if user-end input adjustments are enabled for the current system configuration. */
    bool IsWritable() const;

    /**
     * @brief Flag notifying the UI hierarchy that input fields contain uncommitted adjustments.
     * @param dirty Sets flag status when true, clears it when false.
     */
    void MarkAsDirty(bool dirty = true);

    /** @brief Checks if any uncommitted adjustments exist inside user configuration edits. */
    bool IsDirty() const;

protected:
    /**
     * @brief Specialized initialization constructor for downstream implementation setups.
     * @param parent Pointer to parent wxWidgets layout window panel object context.
     * @param param Instantiated parameter settings structural storage reference map instance.
     * @param hasResetButton Flag detailing if the reset button should be rendered.
     */
    ParameterWidget(
        wxWindow* parent,
        const std::shared_ptr<ParameterInfo>& param,
        bool hasResetButton = true
    );

    // Constructs structural title information wrappers above implementation scopes.
    wxPanel* CreateHeader() override;

	// Hooks into the update sequence to apply parameter data changes down to implementation scopes.
    virtual void InitializeParameter() = 0;

	// Hooks into the update sequence to apply parameter data changes down to implementation scopes.
    virtual void UpdateImpl(const std::shared_ptr<ParameterInfo>& param) = 0;

    /**
     * @brief Pure Virtual Factory Method creating specific interactive data input controls.
     * @return Generated wxWindow element handle (e.g., wxTextCtrl, wxSlider, wxCheckBox).
     */
    virtual wxWindow* CreateValueControl() = 0;

    /**
     * @brief Defers value changes to protect backend data pipes from intermediate keyboard updates.
     * @param event Encapsulated data variant container package mapping to hardware properties.
     * @param milliseconds Duration window defining structural update block intervals.
     */
    void StoreLatestSetParamEvent(const ParameterChangedEvent& event, int milliseconds);

    // Resolves active operational constraints and applies modifications down to lower interfaces.
    void DoSetAccessMode();

    // Hook enabling custom styles when switching inputs over into Read/Write interactions.
    virtual void RWModeImpl() = 0;

    // Hook enabling visual safety block configurations when entering Read-Only conditions.
    virtual void ReadOnlyModeImpl() = 0;

    // Builds hover-based tooltip context descriptions parsing backend data profiles.
    void SetDescriptionImpl() override;

    /**
     * @brief Pure Virtual callback routing value modification changes onto target configuration points.
     * @param event Core wxWidgets UI command framework transmission channel.
     */
    virtual void OnSetParameter(wxCommandEvent& event) = 0;

    //-----------------------------------------------------------------------------
    // Events
    //-----------------------------------------------------------------------------

    // Intercepts general clicking maps to safely catch tracking frame highlights.
    void OnSpecializedWidgetClick(wxMouseEvent& event) override;

private:
    // Orchestrates structural dynamic UI generation and base tracking behaviors.
    void InitializeImpl() override;

    virtual void DoSetValue() = 0;

    void DoSetParameterName();

    void DoSetUnit();

    void DoSetImposedAccessMode();

    void DoMarkAsDirty();

    void RWMode();

    void ReadOnlyMode();

    // Dispatches the cached user inputs across hardware links once timers complete.
    void ProcessLatestSetParamEvent();


    //-----------------------------------------------------------------------------
    // Events
    //-----------------------------------------------------------------------------

	// Catches reset button clicks to trigger parameter value reversion to defaults.
    void OnResetParameter(wxCommandEvent& event);

	// Handles deferred event dispatches to protect backend data pipes from intermediate keyboard updates.
    void OnDebounceTimer(wxTimerEvent&);
    
protected:
    // Layout wrapper holding base parameter labels.
    wxPanel* m_parameterHeader;

private:
    // Explicit data encoding category (Integer, Float, String).
    DataType m_valueType;

    // Backend tracking key index.
    wxString m_parameterId;

    // Group allocation container reference.
    wxString m_categoryId;

    // Tracking presence profile for hardware tracking values.
    bool m_hasResetButton;

    // Active soft access baseline permission level.
    AccessMode m_accessMode{ AccessMode::WRITEONLY };

    // Rigid explicit hard restriction override layer.
    AccessMode m_imposedAccessMode{ AccessMode::WRITEONLY };

    // Interactive string storage displaying tooltips explaining status blocks.
    std::optional<wxString> m_imposedAccessModeReason;

    // Parsed name identifier.
    wxString m_paramName;

    // Value unit metrics.
    wxString m_unit;

    // Derived internal interface target handle tracking user selection updates.
    wxWindow* m_valueControl{ nullptr };

    // Positioning alignment grids managing value parameters.
    wxBoxSizer* m_valueControlSizer{ nullptr };

    // Target icon handle triggers default parameters values.
    wxBitmapButton* m_resetButton{ nullptr };

    // Compiled computed permission tracking register flag.
    bool m_isWritable{ true };

    // Changes-pending indicator tracking local user entries.
    bool m_isDirty{ false };

    // Indicators detailing forced system conditions lock blocks.
    wxGenericStaticBitmap* m_imposedAccessModeBmp{ nullptr };

    // Text rendering box displaying component classifications.
    wxStaticText* m_parameterNameTxt{ nullptr };

    // Structural unit metrics box layout point.
    wxStaticText* m_unitTxt{ nullptr };

    // Cached target message sequence pointer.
    ParameterChangedEvent* m_lastSetParamEvent{ nullptr };

    // Timer control tracking execution loops during fast text input entries.
    wxTimer m_debounceTimer;
};
