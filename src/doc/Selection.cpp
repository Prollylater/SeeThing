#include "Selection.h"

#include "Edits.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace doc
{
    namespace
    {
        Shape emptyShape(int width, int height)
        {
            return {core::Image<std::uint8_t>(width, height, 1, core::ColorSpace::Mask), {}};
        }

        void include(Shape &shape, int x, int y)
        {
            shape.mask(x, y) = 255;
            shape.bounds = core::unite(shape.bounds, {x, y, 1, 1});
        }
    }

    const char *selectModeName(SelectMode mode)
    {
        switch (mode)
        {
        case SelectMode::Replace: return "Replace";
        case SelectMode::Add: return "Add";
        case SelectMode::Subtract: return "Subtract";
        case SelectMode::Intersect: return "Intersect";
        }
        return "?";
    }

    Shape rectShape(int width, int height, core::Recti rect)
    {
        Shape shape = emptyShape(width, height);
        const core::Recti clipped = core::intersect(rect, {0, 0, width, height});
        for (int y = clipped.y; y < clipped.bottom(); ++y)
            std::fill_n(shape.mask.row(y) + clipped.x, clipped.w, std::uint8_t(255));
        shape.bounds = clipped;
        return shape;
    }

    Shape ellipseShape(int width, int height, core::Recti box)
    {
        Shape shape = emptyShape(width, height);
        if (box.empty())
            return shape;
        const float rx = box.w * 0.5f, ry = box.h * 0.5f; // ellipse radii
        const float cx = box.x + rx, cy = box.y + ry; // ellipse center
        const core::Recti clipped = core::intersect(box, {0, 0, width, height});
        for (int y = clipped.y; y < clipped.bottom(); ++y)
            for (int x = clipped.x; x < clipped.right(); ++x)
            {
                const float dx = (x + 0.5f - cx) / rx, dy = (y + 0.5f - cy) / ry; // offset from the center, in radii
                if (dx * dx + dy * dy <= 1.0f)
                    include(shape, x, y);
            }
        return shape;
    }

    // Scanline: on each row, the edges crossing the pixel centres' line, sorted, fill between pairs
    Shape polygonShape(int width, int height, std::span<const core::Vec2f> points)
    {
        Shape shape = emptyShape(width, height);
        if (points.size() < 3)
            return shape;
        float top = points[0].y, bottom = points[0].y;
        for (core::Vec2f point : points)
        {
            top = std::min(top, point.y);
            bottom = std::max(bottom, point.y);
        }
        std::vector<float> crossings;
        const int y0 = std::max(0, int(std::floor(top))), y1 = std::min(height - 1, int(std::ceil(bottom)));
        for (int y = y0; y <= y1; ++y)
        {
            const float cy = y + 0.5f; // y of the row's pixel centers
            crossings.clear();
            for (std::size_t i = 0; i < points.size(); ++i)
            {
                const core::Vec2f from = points[i], to = points[(i + 1) % points.size()];
                // Half-open in y, so a vertex on the line counts once
                if ((from.y <= cy) != (to.y <= cy))
                    crossings.push_back(from.x + (cy - from.y) / (to.y - from.y) * (to.x - from.x));
            }
            std::sort(crossings.begin(), crossings.end());
            for (std::size_t i = 0; i + 1 < crossings.size(); i += 2)
            {
                // Centres x + 0.5 in [left, right)
                const int xa = std::max(0, int(std::ceil(crossings[i] - 0.5f)));
                const int xb = std::min(width, int(std::ceil(crossings[i + 1] - 0.5f)));
                for (int x = xa; x < xb; ++x)
                    include(shape, x, y);
            }
        }
        return shape;
    }

    bool applySelection(Document &doc, const Shape &shape, SelectMode mode, std::string label)
    {
        if (shape.mask.width() != doc.width() || shape.mask.height() != doc.height())
            return false;
        PixelEditRecorder rec(doc, selectionTarget);
        // Add and Subtract change only the shape's pixels; the others can change everywhere
        const bool local = mode == SelectMode::Add || mode == SelectMode::Subtract;
        const core::Recti area = local ? shape.bounds : doc.bounds();
        rec.touch(area);
        core::Image<std::uint8_t> &sel = rec.image();
        for (int y = area.y; y < area.bottom(); ++y)
            for (int x = area.x; x < area.right(); ++x)
            {
                const std::uint8_t in = shape.mask(x, y);
                std::uint8_t &value = sel(x, y);
                switch (mode)
                {
                case SelectMode::Replace: value = in; break;
                case SelectMode::Add: value = std::max(value, in); break;
                case SelectMode::Subtract: value = std::min(value, std::uint8_t(255 - in)); break;
                case SelectMode::Intersect: value = std::min(value, in); break;
                }
            }
        return rec.commit(std::move(label));
    }
}
