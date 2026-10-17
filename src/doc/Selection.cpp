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

        void include(Shape &s, int x, int y)
        {
            s.mask(x, y) = 255;
            s.bounds = core::unite(s.bounds, {x, y, 1, 1});
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
        Shape s = emptyShape(width, height);
        const core::Recti r = core::intersect(rect, {0, 0, width, height});
        for (int y = r.y; y < r.bottom(); ++y)
            std::fill_n(s.mask.row(y) + r.x, r.w, std::uint8_t(255));
        s.bounds = r;
        return s;
    }

    Shape ellipseShape(int width, int height, core::Recti box)
    {
        Shape s = emptyShape(width, height);
        if (box.empty())
            return s;
        const float rx = box.w * 0.5f, ry = box.h * 0.5f;
        const float cx = box.x + rx, cy = box.y + ry;
        const core::Recti r = core::intersect(box, {0, 0, width, height});
        for (int y = r.y; y < r.bottom(); ++y)
            for (int x = r.x; x < r.right(); ++x)
            {
                const float dx = (x + 0.5f - cx) / rx, dy = (y + 0.5f - cy) / ry;
                if (dx * dx + dy * dy <= 1.0f)
                    include(s, x, y);
            }
        return s;
    }

    // Scanline: on each row, the edges crossing the pixel centres' line, sorted, fill between pairs
    Shape polygonShape(int width, int height, std::span<const core::Vec2f> points)
    {
        Shape s = emptyShape(width, height);
        if (points.size() < 3)
            return s;
        float top = points[0].y, bottom = points[0].y;
        for (core::Vec2f p : points)
        {
            top = std::min(top, p.y);
            bottom = std::max(bottom, p.y);
        }
        std::vector<float> crossings;
        const int y0 = std::max(0, int(std::floor(top))), y1 = std::min(height - 1, int(std::ceil(bottom)));
        for (int y = y0; y <= y1; ++y)
        {
            const float cy = y + 0.5f;
            crossings.clear();
            for (std::size_t i = 0; i < points.size(); ++i)
            {
                const core::Vec2f a = points[i], b = points[(i + 1) % points.size()];
                // Half-open in y, so a vertex on the line counts once
                if ((a.y <= cy) != (b.y <= cy))
                    crossings.push_back(a.x + (cy - a.y) / (b.y - a.y) * (b.x - a.x));
            }
            std::sort(crossings.begin(), crossings.end());
            for (std::size_t i = 0; i + 1 < crossings.size(); i += 2)
            {
                // Centres x + 0.5 in [left, right)
                const int xa = std::max(0, int(std::ceil(crossings[i] - 0.5f)));
                const int xb = std::min(width, int(std::ceil(crossings[i + 1] - 0.5f)));
                for (int x = xa; x < xb; ++x)
                    include(s, x, y);
            }
        }
        return s;
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
                std::uint8_t &v = sel(x, y);
                switch (mode)
                {
                case SelectMode::Replace: v = in; break;
                case SelectMode::Add: v = std::max(v, in); break;
                case SelectMode::Subtract: v = std::min(v, std::uint8_t(255 - in)); break;
                case SelectMode::Intersect: v = std::min(v, in); break;
                }
            }
        return rec.commit(std::move(label));
    }
}
