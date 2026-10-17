#ifndef UI_SELECTIONOVERLAY_H
#define UI_SELECTIONOVERLAY_H

#include "CanvasView.h"
#include "CanvasWidget.h"
#include "gpu/Renderer.h"

namespace ui
{
    // Over a canvas drawn this frame: dims what is outside the selection and runs marching ants on its edge.
    void drawSelectionOverlay(gpu::Renderer &renderer, const gpu::Texture &mask, const CanvasView &view, const CanvasInput &canvas);
}

#endif
