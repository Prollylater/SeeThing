#ifndef UI_CANVASWIDGET_H
#define UI_CANVASWIDGET_H

#include "CanvasView.h"
#include "gpu/Texture.h"

namespace ui
{
    struct CanvasInput
    {
        bool hovered = false;
        bool painting = false;      // left button held on the canvas, not panning
        bool strokeStarted = false; // painting began this frame
        bool strokeEnded = false;
        core::Vec2f mouse;
        // The canvas area on screen, for overlays
        core::Vec2f origin, size;
    };

    // Toolbar (Fit, 100%, zoom, cursor position) and the image below it. Wheel zooms around the cursor;
    // right or middle drag, or Space + left drag, pans; leftPans makes any left drag pan. Fits itself when the image size changes.
    CanvasInput canvasWidget(const char *id, const gpu::Texture &tex, CanvasView &view, bool leftPans = false);
}

#endif
