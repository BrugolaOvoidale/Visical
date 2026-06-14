#pragma once
#include <gui_elements/base_widget/BaseWidget.hpp>
#include "BoardWidgetDTO.hpp"


/**
 * @brief A graphic component handling the display for an calibrarion board.
 */
class BoardWidget : public BaseWidget
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    /**
     * @brief Constructs a new Board Widget.
     * @param parent Pointer to the parent window layer.
     * @param boardDTO Handle to structural info describing this specific board instance.
     */
    BoardWidget(
        wxWindow* parent,
        const BoardWidgetDTO& boardDTO
    );

    ~BoardWidget() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /**
     * @brief Refreshes the widget's layout and values using an updated data state.
     * @param boardDTO The new data containing updated status and structural configurations.
     */
    void Update(const BoardWidgetDTO& boardDTO);

    /** @brief Returns the unique identifier corresponding to the underlying board model. */
    std::uint32_t GetBoardId() const;

    /**
     * @brief Updates the local user note string.
     */
    void SetNote(const wxString& note);

    /**
     * @brief Sets the internal status configuration for detection and evaluations.
     * @param detCode Status enumeration indicating target recognition states.
     * @param evalCode Status enumeration indicating analytical validation metrics.
     */
    void SetStatus(
        BoardWidgetDTO::BoardStatus detCode,
        BoardWidgetDTO::EvaluationStatus evalCode
    );

    /**
     * @brief Flags whether this visual item requires some sort of attention.
     * @param needAttention True to display attention markers.
     */
    void SetAttention(bool needAttention);

    /** @brief Checks if this component is actively displaying an attention marker. */
    bool NeedAttention() const;

private:
    wxPanel* CreateHeader() override;

    void InitializeImpl() override;

    void SetDescriptionImpl() override;

    void DoSetNote();

    void DoSetStatus();

    void DoSetAttention();


    //-----------------------------------------------------------------------------
    // Events
    //-----------------------------------------------------------------------------

    void OnSpecializedWidgetClick(wxMouseEvent& event) override;

private:
    // Immutable identifier tracking the bound model instance.
    const std::uint32_t m_boardId;

    // Native label displaying custom developer or configuration logs.
    wxStaticText* m_noteItem{ nullptr };

    // Cached target note text body.
    wxString m_note;

    // Internal state indicating if attention marker is active.
    bool m_hasAttention{ false };

    // Internal mirror tracking structural detection visibility.
    BoardWidgetDTO::BoardStatus m_boardStatus;

    // Internal mirror tracking functional metric quality results.
    BoardWidgetDTO::EvaluationStatus m_evalStatus;

    // UI frame representing detection output strings.
    wxStaticText* m_detectionStatus{ nullptr };

    // UI frame representing metric validation results.
    wxStaticText* m_evaluationStatus{ nullptr };

    // UI attention marker.
    wxStaticText* m_attentionText{ nullptr };
};
