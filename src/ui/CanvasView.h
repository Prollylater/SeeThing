#ifndef UI_CANVASVIEW_H
#define UI_CANVASVIEW_H

#include "core/Geometry.h"

#include <algorithm>
#include <cmath>

namespace ui
{
    // Screen <-> image mapping of a canvas: screen = origin + pan + image * zoom. The one transform tools and overlays use.
    struct CanvasView
    {
        static constexpr float minZoom = 1.0f / 32.0f;
        static constexpr float maxZoom = 64.0f;

        core::Vec2f pan{0, 0};
        float zoom = 1.0f;
        // The image size the view was last fitted to; a new size refits
        core::Vec2f fittedSize{-1, -1};
        enum class Request { None, Fit, ActualSize } request = Request::None;

        core::Vec2f toScreen(core::Vec2f image, core::Vec2f origin) const { return origin + pan + image * zoom; }
        core::Vec2f toImage(core::Vec2f screen, core::Vec2f origin) const { return (screen - origin - pan) / zoom; }

        // Keeps the image point under the anchor where it is
        void zoomAround(core::Vec2f anchor, core::Vec2f origin, float newZoom)
        {
            const core::Vec2f fixed = toImage(anchor, origin);
            zoom = std::clamp(newZoom, minZoom, maxZoom);
            pan = anchor - origin - fixed * zoom;
        }

        void center(core::Vec2f imageSize, core::Vec2f viewSize)
        {
            pan = {std::round((viewSize.x - imageSize.x * zoom) * 0.5f), std::round((viewSize.y - imageSize.y * zoom) * 0.5f)};
        }

        // Whole image visible, never enlarged past 100%
        void fit(core::Vec2f imageSize, core::Vec2f viewSize)
        {
            if (imageSize.x > 0 && imageSize.y > 0 && viewSize.x > 0 && viewSize.y > 0)
                zoom = std::clamp(std::min({viewSize.x / imageSize.x, viewSize.y / imageSize.y, 1.0f}), minZoom, maxZoom);
            center(imageSize, viewSize);
            fittedSize = imageSize;
        }

        void actualSize(core::Vec2f imageSize, core::Vec2f viewSize)
        {
            zoom = 1.0f;
            center(imageSize, viewSize);
        }

        // A wheel notch is a quarter power of two; whole notches snap to that grid, so 100% is always reachable
        static float stepZoom(float zoom, float wheel)
        {
            float level = std::log2(zoom) * 4.0f + wheel;
            if (std::fabs(wheel) >= 1.0f)
                level = std::round(level);
            return std::clamp(std::exp2(level / 4.0f), minZoom, maxZoom);
        }
    };
}

#endif
