#include "ImagePanel.hpp"
#include <wx/dcbuffer.h>
#include <gui_elements/persistent_tooltip/PersistentToolTip.hpp>
#include "PixelEvent.hpp"


ImagePanel::ImagePanel(wxWindow* parent)
    : wxPanel(parent),
    m_displayMode(FIT_ASPECT_RATIO),
    m_currDisplayMode(FIT_ASPECT_RATIO),
    m_scale(1.0),
    m_bitmapOffset(0, 0),
    m_dragging(false),
    m_lastMousePos(0, 0)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);

    m_pixelTooltip = new PersistentToolTip(this, wxEmptyString);

    Bind(wxEVT_PAINT, &ImagePanel::OnPaint, this);
    Bind(wxEVT_SIZE, &ImagePanel::OnSize, this);
    Bind(wxEVT_MOTION, &ImagePanel::OnMouseMotion, this);
    Bind(wxEVT_MOUSEWHEEL, &ImagePanel::OnMouseWheel, this);
    Bind(wxEVT_LEFT_DOWN, &ImagePanel::OnMouseLeftDown, this);
    Bind(wxEVT_LEFT_UP, &ImagePanel::OnMouseLeftUp, this);
}

////////////////////////////////////////////////////////////////

void ImagePanel::SetImage(const wxImage& img)
{
    if (!img.IsOk()) return;
    m_image = img;
    SetPart(0, 0, img.GetHeight() - 1, img.GetWidth() - 1, false);
    Refresh(false);
}

void ImagePanel::SetPixelToolTip(const wxString& tip)
{
    m_pixelTooltip->ShowAt(ClientToScreen(ScreenToClient(wxGetMousePosition())), tip);
}

void ImagePanel::UnsetPixelToolTip()
{
    m_pixelTooltip->Hide();
}

void ImagePanel::ClearDisplay()
{
    m_image = wxImage();
    m_partImage = wxImage();
    m_cachedBitmap = wxBitmap();
    m_bitmapOffset = wxPoint(0, 0);
    m_scale = 1.0;
    m_currDisplayMode = m_displayMode;
    m_dragging = false;
    Refresh();
}

void ImagePanel::SetPart(int row1, int col1, int row2, int col2, bool refresh)
{
    if (!m_image.IsOk()) return;
    if (row1 > row2) std::swap(row1, row2);
    if (col1 > col2) std::swap(col1, col2);
    row1 = std::max(0, std::min(row1, m_image.GetHeight() - 1));
    row2 = std::max(0, std::min(row2, m_image.GetHeight() - 1));
    col1 = std::max(0, std::min(col1, m_image.GetWidth() - 1));
    col2 = std::max(0, std::min(col2, m_image.GetWidth() - 1));

    m_partOrigin = wxPoint(col1, row1);
    wxRect partRect(col1, row1, col2 - col1 + 1, row2 - row1 + 1);

    // Only reset zoom/pan when the visible region changes size
    const bool sizeChanged = !m_partImage.IsOk()
        || m_partImage.GetWidth() != partRect.width
        || m_partImage.GetHeight() != partRect.height;

    m_partImage = m_image.GetSubImage(partRect);

    if (sizeChanged)
    {
        if (m_currDisplayMode == CUSTOM_ZOOM)
            m_currDisplayMode = m_displayMode;

        m_scale = 1.0;
        m_bitmapOffset = wxPoint(0, 0);
    }
    // else: same size, keep m_scale, m_bitmapOffset, m_currDisplayMode intact

    UpdateScaledBitmap();
    if (refresh) Refresh();
}

void ImagePanel::SetDisplayMode(DisplayMode mode)
{
    m_displayMode = mode;
    m_currDisplayMode = m_displayMode;
    UpdateScaledBitmap();
    Refresh();
}

////////////////////////////////////////////////////////////////

void ImagePanel::UpdateScaledBitmap()
{
    if (!m_partImage.IsOk()) { m_cachedBitmap = wxBitmap(); return; }

    wxSize client = GetClientSize();
    if (client.x <= 0 || client.y <= 0)
    {
        m_cachedBitmap = wxBitmap(m_partImage);
        m_scale = 1.0;
        m_bitmapOffset = m_tileDrawOffset = wxPoint(0, 0);

        return;
    }

    const int imgW = m_partImage.GetWidth();
    const int imgH = m_partImage.GetHeight();

    if (m_currDisplayMode == FIT_TO_PANEL)
    {
        double scaleX = static_cast<double>(client.x) / imgW;
        double scaleY = static_cast<double>(client.y) / imgH;

        m_scale = (scaleX + scaleY) / 2.0;
        m_cachedBitmap = wxBitmap(m_partImage.Scale(client.x, client.y, wxIMAGE_QUALITY_NEAREST));
        m_bitmapOffset = m_tileDrawOffset = wxPoint(0, 0);

        return;
    }

    // 1. Resolve scale
    const double minScale = GetFitScale();
    if (m_currDisplayMode == ONE_TO_ONE)       m_scale = 1.0;
    else if (m_currDisplayMode == FIT_ASPECT_RATIO) m_scale = minScale;
    else if (m_scale < minScale)                m_scale = minScale;

    const int fullW = std::max(1, static_cast<int>(imgW * m_scale + 0.5));
    const int fullH = std::max(1, static_cast<int>(imgH * m_scale + 0.5));

    // 2. Clamp logical offset (as if the full scaled image existed)
    auto clampOff = [](int full, int client, int off) -> int
        {
        if (full <= client) return (client - full) / 2;          // center
        return std::max(client - full, std::min(0, off));        // pan bounds
        };

    m_bitmapOffset.x = clampOff(fullW, client.x, m_bitmapOffset.x);
    m_bitmapOffset.y = clampOff(fullH, client.y, m_bitmapOffset.y);

    // 3. Visible rect in full-scaled-image space
    const int visX0 = std::max(0, -m_bitmapOffset.x);
    const int visY0 = std::max(0, -m_bitmapOffset.y);
    const int visX1 = std::min(fullW, visX0 + client.x);
    const int visY1 = std::min(fullH, visY0 + client.y);

    // 4. Source tile: floor left/top, ceil right/bottom (+1 margin)
    // This guarantees the tile always covers the whole visible region.
    const double inv = 1.0 / m_scale;
    const int srcX = std::max(0, static_cast<int>(std::floor(visX0 * inv)));
    const int srcY = std::max(0, static_cast<int>(std::floor(visY0 * inv)));
    const int srcX1 = std::min(imgW, static_cast<int>(std::ceil(visX1 * inv)) + 1);
    const int srcY1 = std::min(imgH, static_cast<int>(std::ceil(visY1 * inv)) + 1);

    // 5. Output rect: derived from the SAME floor/ceil so draw pos and
    // bitmap size are always consistent, no rounding gaps
    const int drawX = static_cast<int>(std::floor(m_bitmapOffset.x + srcX * m_scale));
    const int drawY = static_cast<int>(std::floor(m_bitmapOffset.y + srcY * m_scale));
    const int endX = static_cast<int>(std::ceil(m_bitmapOffset.x + srcX1 * m_scale));
    const int endY = static_cast<int>(std::ceil(m_bitmapOffset.y + srcY1 * m_scale));
    const int outW = std::max(1, endX - drawX);
    const int outH = std::max(1, endY - drawY);

    // 6. Scale only the visible tile
    const bool whole = (srcX == 0 && srcY == 0 && srcX1 == imgW && srcY1 == imgH);
    wxImage tile = whole ? m_partImage
        : m_partImage.GetSubImage(wxRect(srcX, srcY, srcX1 - srcX, srcY1 - srcY));

    m_cachedBitmap = wxBitmap(
        (std::fabs(m_scale - 1.0) < 1e-5) ? tile.Copy()
        : tile.Scale(outW, outH, wxIMAGE_QUALITY_NEAREST));

    m_tileDrawOffset = wxPoint(drawX, drawY);
}

double ImagePanel::GetFitScale() const
{
    if (!m_partImage.IsOk()) return 1.0;

    wxSize client = GetClientSize();
    if (client.x <= 0 || client.y <= 0) return 1.0;

    double scaleX = static_cast<double>(client.x) / m_partImage.GetWidth();
    double scaleY = static_cast<double>(client.y) / m_partImage.GetHeight();

    // We use min() because that represents the "Fit to Screen" scale 
    // where the whole image is visible.
    return std::min(scaleX, scaleY);
}

bool ImagePanel::PanelToImage(int panelX, int panelY, int& imgX, int& imgY) const
{
    if (!m_cachedBitmap.IsOk() || !m_partImage.IsOk() || m_scale <= 0.0)
        return false;

    double bx = (panelX - m_bitmapOffset.x) / m_scale;
    double by = (panelY - m_bitmapOffset.y) / m_scale;
    int px = static_cast<int>(bx);
    int py = static_cast<int>(by);

    if (px < 0 || py < 0 ||
        px >= m_partImage.GetWidth() ||
        py >= m_partImage.GetHeight())
        return false;

    imgX = px + m_partOrigin.x;
    imgY = py + m_partOrigin.y;

    return true;
}

void ImagePanel::OnSize(wxSizeEvent&)
{
    UpdateScaledBitmap();
    Refresh();
}

void ImagePanel::OnPaint(wxPaintEvent&)
{
    wxAutoBufferedPaintDC dc(this);

    if (!m_cachedBitmap.IsOk())
    {
        dc.Clear();   // nothing to show  clear once and bail
        return;
    }

    const wxSize  cs = GetClientSize();
    const int     bx = m_tileDrawOffset.x;
    const int     by = m_tileDrawOffset.y;
    const int     bw = m_cachedBitmap.GetWidth();
    const int     bh = m_cachedBitmap.GetHeight();

    // Erase only the four margins that the bitmap does NOT cover.
    // When the image fills the panel these rects are empty -> no clear at all.
    dc.SetBrush(wxBrush(GetBackgroundColour()));
    dc.SetPen(*wxTRANSPARENT_PEN);

    // top
    if (by > 0)
        dc.DrawRectangle(0, 0, cs.x, by);
    // bottom
    if (by + bh < cs.y)
        dc.DrawRectangle(0, by + bh, cs.x, cs.y - (by + bh));
    // left  (only the band beside the bitmap, not the corners)
    if (bx > 0)
        dc.DrawRectangle(0, by, bx, bh);
    // right
    if (bx + bw < cs.x)
        dc.DrawRectangle(bx + bw, by, cs.x - (bx + bw), bh);

    dc.DrawBitmap(m_cachedBitmap, bx, by, false);
}

void ImagePanel::OnMouseMotion(wxMouseEvent& event)
{
    if (m_dragging)
    {
        wxPoint current = event.GetPosition();
        wxPoint delta = current - m_lastMousePos;
        m_bitmapOffset.x += delta.x;
        m_bitmapOffset.y += delta.y;
        m_lastMousePos = current;
        UpdateScaledBitmap();
        Refresh(false);
        return;
    }

    event.Skip(); // allow other handlers if any

    if (!event.ControlDown())
    {
        m_pixelTooltip->Hide();
        return;
    }
    if (!m_image.IsOk()) return;

    int imgX, imgY;
    if (!PanelToImage(event.GetX(), event.GetY(), imgX, imgY))
    {
        m_pixelTooltip->Hide();
        return;
    }

    PixelEvent hoverEvent(GUI_CTRL_HOVER_PIXEL, GetId());
    hoverEvent.SetPixel(imgX, imgY);
    ProcessEvent(hoverEvent);
}

void ImagePanel::OnMouseWheel(wxMouseEvent& event)
{
    if (!event.ControlDown() || !m_partImage.IsOk() || m_scale <= 0.0) {
        event.Skip();
        return;
    }

    wxPoint mousePos = event.GetPosition();

    // Image coordinate (floating-point, can be outside the part - this enables panning on zoom)
    double imgRelX = (mousePos.x - m_bitmapOffset.x) / m_scale;
    double imgRelY = (mousePos.y - m_bitmapOffset.y) / m_scale;

    int delta = event.GetWheelRotation();
    if (delta == 0)
    {
        event.Skip();
        return;
    }

    // Smooth zoom: 25% per wheel notch (handles fast scrolling via steps)
    double steps = static_cast<double>(delta) / event.GetWheelDelta();
    double factor = std::pow(1.25, steps);
    double newScale = m_scale * factor;

    // Reasonable limits
    newScale = std::max(GetFitScale(), std::min(50.0, newScale));

    // Keep the point under the mouse fixed after zoom
    double newOffsetX = mousePos.x - imgRelX * newScale;
    double newOffsetY = mousePos.y - imgRelY * newScale;

    m_scale = newScale;
    m_bitmapOffset.x = static_cast<int>(std::round(newOffsetX));
    m_bitmapOffset.y = static_cast<int>(std::round(newOffsetY));

    if (m_currDisplayMode != CUSTOM_ZOOM)
    {
        m_currDisplayMode = CUSTOM_ZOOM;
    }

    UpdateScaledBitmap();   // rebuilds cached bitmap at new scale, clamps offset
    Refresh(false);
    // Do NOT Skip() - we consumed the wheel event
}

void ImagePanel::OnMouseLeftDown(wxMouseEvent& event)
{
    if (event.ControlDown())
    {
        event.Skip();   // let tooltip / other CTRL actions work
        return;
    }
    if (!m_cachedBitmap.IsOk())
    {
        event.Skip();
        return;
    }

    m_dragging = true;
    m_lastMousePos = event.GetPosition();
    CaptureMouse();
}

void ImagePanel::OnMouseLeftUp(wxMouseEvent& event)
{
    if (m_dragging)
    {
        m_dragging = false;
        if (HasCapture()) ReleaseMouse();
    }

    event.Skip();
}