#pragma once
#include <wx/panel.h>
#include <wx/image.h>
#include <wx/bitmap.h>


// Forward declarations
class PersistentToolTip;


/**
 * @brief A custom wxWidgets panel dedicated to rendering, zooming, and panning images.
 *
 * This control handles multiple layout policies (stretching, aspect ratio locking, 1:1 matching)
 * and enables interactive mouse behavior such as scroll-wheel zooming and click-and-drag panning.
 * It tracks sub-regions (ROI) of an image natively via a clipping/part mechanism.
 */
class ImagePanel : public wxPanel
{
public:
    //-----------------------------------------------------------------------------
    // Enums
    //-----------------------------------------------------------------------------

/**
     * @brief Layout scaling policies for displaying the source image inside the panel canvas.
     */
    enum DisplayMode {
        FIT_TO_PANEL,     ///< Stretches the image to fully fill the panel canvas, ignoring original aspect ratio.
        ONE_TO_ONE,       ///< Renders the image at its absolute pixel-native scale. Centered if smaller than panel.
        FIT_ASPECT_RATIO, ///< Scales the image to fit the panel boundaries while preserving its original aspect ratio.
        CUSTOM_ZOOM       ///< Variable scaling triggered dynamically by manual mouse scroll zooming. Resets on new images.
    };


    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    /**
     * @brief Constructs an ImagePanel canvas.
     * @param parent Pointer to the parent window layout.
     */
    ImagePanel(wxWindow* parent);

    ~ImagePanel() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /**
     * @brief Assigns a new image to be managed and displayed by the panel.
     * @param img The source image dataset.
     * @note This clears any previously set region sub-parts and resets the layout mode back to defaults.
     */
    void SetImage(const wxImage& img);

    /**
     * @brief Activates a dynamic hovering tooltip that displays specialized per-pixel coordinates.
     * @param tip The tooltip format string or descriptive label text.
     */
    void SetPixelToolTip(const wxString& tip);

    /**
     * @brief Disables and tears down the active pixel tracking hover tooltip.
     */
    void UnsetPixelToolTip();

    /**
     * @brief Clears out internal image caches and resets the canvas rendering area to blank.
     */
    void ClearDisplay();

    /**
     * @brief Locks the view window to a cropped sub-bounding box (Region of Interest) inside the source image.
     * @param row1 Starting row pixel boundary.
     * @param col1 Starting column pixel boundary.
     * @param row2 Ending row pixel boundary.
     * @param col2 Ending column pixel boundary.
     * @param refresh If true, explicitly triggers a window update event loop redraw.
     */
    void SetPart(int row1, int col1, int row2, int col2, bool refresh = true);

    /**
     * @brief Overrides the structural scaling behavior applied to the displayed bitmap canvas.
     * @param mode Selected formatting layout rule.
     */
    void SetDisplayMode(DisplayMode mode);

private:
    // Re-allocates or stretches the inner wxBitmap cache following size updates or zoom adjustments.
    void UpdateScaledBitmap();

    // Helper to evaluate the exact uniform scale scalar needed to achieve aspect ratio bounds fitting.
    double GetFitScale() const;

    /**
     * @brief Maps raw local display panel coordinates back to the underlying image array indices.
     * @param panelX Screen X coordinate inside the widget canvas.
     * @param panelY Screen Y coordinate inside the widget canvas.
     * @param[out] imgX Reference to receive mapped original Image X pixel.
     * @param[out] imgY Reference to receive mapped original Image Y pixel.
     * @return true if the requested coordinate target lies within actual valid image pixel boundaries.
     */
    bool PanelToImage(int panelX, int panelY, int& imgX, int& imgY) const;


    //-----------------------------------------------------------------------------
    // Events
    //-----------------------------------------------------------------------------

    void OnSize(wxSizeEvent&);

    void OnPaint(wxPaintEvent&);

    void OnMouseMotion(wxMouseEvent& event);

    void OnMouseWheel(wxMouseEvent& event);

    void OnMouseLeftDown(wxMouseEvent& event);

    void OnMouseLeftUp(wxMouseEvent& event);

private:
    // The original unscaled raw source image.
    wxImage m_image;

    // Current cropped sub-region image workspace extracted via SetPart.
    wxImage m_partImage;

    // GPU/System optimized canvas wrapper matched to active display limits.
    wxBitmap m_cachedBitmap;

    // User-requested layout rules framework state tracker.
    DisplayMode m_displayMode;

    // Effective active rendering rules tracking state.
    DisplayMode m_currDisplayMode;

    // Current multiplier scale mapping source pixels into UI display space.
    double m_scale;

    // Positional adjustment offset used when centering smaller images.
    wxPoint m_bitmapOffset;

    // Offset of the current sub-region relative to the original source image top-left.
    wxPoint m_partOrigin;

    // UI overlay asset rendering spatial inspection information.
    PersistentToolTip* m_pixelTooltip{ nullptr };

    // Internal adjustment tracking layout alignment offsets.
    wxPoint m_tileDrawOffset;

    // Track variable flag checking if interactive click-panning is engaged.
    bool m_dragging;

    // Stored cursor coordinate state used to compute relative pan deltas.
    wxPoint m_lastMousePos;
};