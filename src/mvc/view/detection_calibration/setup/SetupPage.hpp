#pragma once
#include <memory>
#include <wx/panel.h>
#include "../../BaseView.hpp"


// Forward declarations
class ParameterInfo;
class ParameterWidgetList;
class wxBitmapButton;
class wxStaticText;
class wxRadioButton;
class wxStaticBoxSizer;


/**
 * @brief Configuration view component for defining camera calibration parameters.
 *
 * This panel provides a user interface to configure camera intrinsic matrices and
 * distortion models.
 *
 * It coordinates UI behaviors across three primary operation modes: Automatic, Guided, and Free.
 */
class SetupPage : public BaseView, public wxPanel
{
public:
    //-----------------------------------------------------------------------------
    // Enums
    //-----------------------------------------------------------------------------
    
    /**
     * @brief Vsual containers where parameter subsets are rendered.
     */
    enum class ParameterLocation {
        DISTORTION_MODEL,   ///< The Distortion Model parameter list box.
        CAMERA_INTRINSICS,  ///< The Camera Intrinsics parameter list box.
        CAMERA_MATRIX       ///< The Raw Camera Matrix parameter list box.
    };


    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    /**
     * @brief Constructs the configuration layout panel.
     * @param parent Pointer to the parent window frame or panel.
     * @param winid Unique window identifier.
     * @param pos Physical display position relative to parent coordinates.
     * @param size Initial layout dimensions.
     * @param style Window flag constraints.
     * @param name Internal identifier name for debugging hooks.
     */
    SetupPage(
        wxWindow* parent,
        wxWindowID winid = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxTAB_TRAVERSAL | wxNO_BORDER,
        const wxString& name = wxASCII_STR(wxPanelNameStr)
    );
    
    ~SetupPage() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /**
     * @brief Populates parameters inside a specific UI sub-panel.
     * @param params A collection of parameter tracking objects.
     * @param where Destination sub-panel section.
     */
    void SetParameters(
        const std::vector<std::shared_ptr<ParameterInfo>>& params,
        ParameterLocation where
    );

    /**
     * @brief Dispatches updates to sync an modified parameter instance with its UI display widget.
     * @param param Shared pointer to the modified property node.
     */
    void UpdateParameter(const std::shared_ptr<ParameterInfo>& param);

    /**
     * @brief Visually marks a parameter field to represent unsaved local changes.
     * @param paramId Unique text key string specifying the parameter tracking node.
     * @param isDirty Flag state where true indicates uncommitted/edited modifications.
     */
    void MarkParameterAsDirty(
        const wxString& paramId,
        bool isDirty
    );

    /** @brief Programmatically activates Automatic Mode. */
    void AutomaticMode();

    /** @brief Programmatically activates From Hardware Mode. */
    void FromHardwareMode();

    /** @brief Programmatically activates Free Mode. */
    void FreeMode();

private:
    void CreateTab();


    //-----------------------------------------------------------------------------
    // Events
    //-----------------------------------------------------------------------------

    void OnLoadConfig(wxCommandEvent& event);

    void OnSaveConfig(wxCommandEvent& event);
    
    void OnLoadModelParams(wxCommandEvent& event);

    void OnSaveModelParams(wxCommandEvent& event);

    void OnAutomaticMode(wxCommandEvent& event);

    void OnFromHardwareMode(wxCommandEvent& event);

    void OnFreeMode(wxCommandEvent& event);

private:
    wxBitmapButton* m_loadConfig;
    wxBitmapButton* m_saveConfig;

    wxStaticText* m_modeBriefing;

    wxBitmapButton* m_loadModelParams;
    wxBitmapButton* m_saveModelParams;

    wxRadioButton* m_automaticRadio;

    wxRadioButton* m_fromHardwareRadio;
    wxStaticBoxSizer* m_distModelBox;
    wxStaticBoxSizer* m_camIntrinsicsBox;

    wxRadioButton* m_freeRadio;
    wxStaticBoxSizer* m_camMatrixBox;

    ParameterWidgetList* m_distModelParamsList;
    ParameterWidgetList* m_camIntrinsicsList;
    ParameterWidgetList* m_camMatrixList;
};
