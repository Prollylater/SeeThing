#ifndef DOC_SELECTION_H
#define DOC_SELECTION_H

#include "Document.h"

#include <span>
#include <string>

// Selection shapes and how they combine with the current selection. A pixel is inside a shape when its centre is.
namespace doc
{
    enum class SelectMode
    {
        Replace,
        Add,
        Subtract,
        Intersect,
    };
    const char *selectModeName(SelectMode mode);

    // A document-size R8 mask, 255 inside, and the rectangle holding everything inside
    struct Shape
    {
        core::Image<std::uint8_t> mask;
        core::Recti bounds;
    };

    Shape rectShape(int width, int height, core::Recti rect);
    // The ellipse inscribed in box
    Shape ellipseShape(int width, int height, core::Recti box);
    // Even-odd rule, so a self-crossing lasso leaves its loops out
    Shape polygonShape(int width, int height, std::span<const core::Vec2f> points);

    // One step; false when the selection did not change
    bool applySelection(Document &doc, const Shape &shape, SelectMode mode, std::string label);
}

#endif
