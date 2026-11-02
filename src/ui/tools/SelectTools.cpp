#include "Tool.h"
#include "Tools.h"

#include "IconsCodicons.h"
#include "app/App.h"
#include "doc/Edits.h"
#include "doc/Selection.h"

#include <cmath>
#include <optional>
#include <vector>

namespace ui
{
    std::unique_ptr<Tool> makeHandTool();
    std::unique_ptr<Tool> makePencilTool();
    std::unique_ptr<Tool> makeBrushTool();
    std::unique_ptr<Tool> makeEraserTool();
    std::unique_ptr<Tool> makePickerTool();
    std::unique_ptr<Tool> makeBucketTool();

    namespace
    {
        ImVec2 im(core::Vec2f v) { return {v.x, v.y}; }
        core::Vec2i pixelAt(core::Vec2f point) { return {int(std::floor(point.x)), int(std::floor(point.y))}; }

        // The option row's mode, unless Shift (add), Alt (subtract) or both (intersect) are held at the press
        struct ModeChoice
        {
            doc::SelectMode option = doc::SelectMode::Replace;

            void draw()
            {
                const doc::SelectMode modes[] = {doc::SelectMode::Replace, doc::SelectMode::Add, doc::SelectMode::Subtract, doc::SelectMode::Intersect};
                for (doc::SelectMode mode : modes)
                {
                    ImGui::SameLine();
                    if (ImGui::RadioButton(doc::selectModeName(mode), option == mode))
                        option = mode;
                }
                ImGui::SameLine();
                ImGui::TextDisabled("(?)");
                ImGui::SetItemTooltip("Hold Shift to add, Alt to subtract, both to intersect");
            }

            doc::SelectMode at(const ToolEvent &event) const
            {
                if (event.shift && event.alt)
                    return doc::SelectMode::Intersect;
                if (event.shift)
                    return doc::SelectMode::Add;
                if (event.alt)
                    return doc::SelectMode::Subtract;
                return option;
            }
        };

        // Dark under light: readable on any image
        void outline(ImDrawList *drawList, const std::vector<ImVec2> &points, bool closed)
        {
            const ImDrawFlags flags = closed ? ImDrawFlags_Closed : ImDrawFlags_None;
            drawList->AddPolyline(points.data(), int(points.size()), IM_COL32(0, 0, 0, 200), 3.0f, flags);
            drawList->AddPolyline(points.data(), int(points.size()), IM_COL32(255, 255, 255, 230), 1.0f, flags);
        }
    }

    // One select tool, three shapes: a box (corners snap to pixel edges) for the rectangle and the ellipse,
    // a freehand outline for the lasso
    class SelectTool : public Tool
    {
    public:
        enum class Shape
        {
            Rectangle,
            Ellipse,
            Lasso,
        };

        const char *name() const override { return "Select"; }
        const char *icon() const override
        {
            switch (m_shape)
            {
            case Shape::Ellipse: return ICON_CI_CIRCLE_LARGE;
            case Shape::Lasso: return ICON_CI_EDITOR_LAYOUT;
            case Shape::Rectangle: break;
            }
            return ICON_CI_SELECTION;
        }
        const char *help() const override
        {
            return "Select (M, again to change the shape): drag a rectangle, an ellipse or a freehand lasso; "
                   "a click without dragging deselects";
        }
        ImGuiKey key() const override { return ImGuiKey_M; }
        void cycleVariant() override { m_shape = Shape((int(m_shape) + 1) % 3); }

        void options(ToolContext &) override
        {
            const char *names[] = {"Rectangle", "Ellipse", "Lasso"};
            for (int i = 0; i < 3; ++i)
            {
                if (i > 0)
                    ImGui::SameLine();
                if (ImGui::RadioButton(names[i], int(m_shape) == i))
                    m_shape = Shape(i);
            }
            ImGui::SameLine();
            ImGui::TextDisabled("|");
            m_mode.draw();
        }

        void press(ToolContext &, const ToolEvent &event) override
        {
            m_dragging = true;
            m_modeNow = m_mode.at(event);
            m_from = m_to = event.pos;
            m_points = {event.pos};
        }

        void drag(ToolContext &, const ToolEvent &event) override
        {
            m_to = event.pos;
            // A lasso point every 2 screen pixels is plenty
            const core::Vec2f delta = event.pos - m_points.back();
            if (std::hypot(delta.x, delta.y) * event.zoom >= 2.0f)
                m_points.push_back(event.pos);
        }

        void release(ToolContext &ctx, const ToolEvent &event) override
        {
            drag(ctx, event);
            m_dragging = false;
            doc::Document &doc = ctx.app.document();
            std::optional<doc::Shape> shape;
            const core::Recti box = rect();
            if (m_shape == Shape::Lasso && m_points.size() >= 3)
                shape = doc::polygonShape(doc.width(), doc.height(), m_points);
            else if (m_shape != Shape::Lasso && box.w >= 1 && box.h >= 1)
                shape = m_shape == Shape::Ellipse ? doc::ellipseShape(doc.width(), doc.height(), box)
                                                  : doc::rectShape(doc.width(), doc.height(), box);
            m_points.clear();
            if (!shape)
            {
                if (m_modeNow == doc::SelectMode::Replace)
                    doc::selectNone(doc);
                return;
            }
            const char *names[] = {"Rectangle select", "Ellipse select", "Lasso"};
            doc::applySelection(doc, *shape, m_modeNow, std::string(names[int(m_shape)]) +
                                                            (m_modeNow == doc::SelectMode::Replace ? "" : std::string(" (") + doc::selectModeName(m_modeNow) + ")"));
        }

        void cancel(ToolContext &) override
        {
            m_dragging = false;
            m_points.clear();
        }

        void overlay(ImDrawList *drawList, const CanvasView &view, const CanvasInput &canvas) const override
        {
            if (!m_dragging)
                return;
            std::vector<ImVec2> points;
            if (m_shape == Shape::Lasso)
            {
                for (core::Vec2f point : m_points)
                    points.push_back(im(view.toScreen(point, canvas.origin)));
                if (points.size() >= 2)
                    outline(drawList, points, true);
                return;
            }
            const core::Recti box = rect();
            const core::Vec2f topLeft = view.toScreen({float(box.x), float(box.y)}, canvas.origin);
            const core::Vec2f bottomRight = view.toScreen({float(box.right()), float(box.bottom())}, canvas.origin);
            if (m_shape == Shape::Ellipse)
            {
                const ImVec2 center{(topLeft.x + bottomRight.x) * 0.5f, (topLeft.y + bottomRight.y) * 0.5f};
                const float rx = (bottomRight.x - topLeft.x) * 0.5f, ry = (bottomRight.y - topLeft.y) * 0.5f; // ellipse radii
                for (int i = 0; i < 64; ++i)
                {
                    const float angle = 6.2831853f * float(i) / 64.0f;
                    points.push_back({center.x + rx * std::cos(angle), center.y + ry * std::sin(angle)});
                }
            }
            else
            {
                points = {im(topLeft), {bottomRight.x, topLeft.y}, im(bottomRight), {topLeft.x, bottomRight.y}};
            }
            outline(drawList, points, true);
        }

    private:
        core::Recti rect() const
        {
            const int x0 = int(std::lround(std::min(m_from.x, m_to.x))), x1 = int(std::lround(std::max(m_from.x, m_to.x)));
            const int y0 = int(std::lround(std::min(m_from.y, m_to.y))), y1 = int(std::lround(std::max(m_from.y, m_to.y)));
            return {x0, y0, x1 - x0, y1 - y0};
        }

        Shape m_shape = Shape::Rectangle;
        ModeChoice m_mode;
        doc::SelectMode m_modeNow = doc::SelectMode::Replace;
        bool m_dragging = false;
        core::Vec2f m_from, m_to;
        std::vector<core::Vec2f> m_points;
    };

    class WandTool : public Tool
    {
    public:
        const char *name() const override { return "Magic wand"; }
        const char *icon() const override { return ICON_CI_WAND; }
        const char *help() const override { return "Magic wand (W): selects the region grown from the click, on the active layer"; }
        ImGuiKey key() const override { return ImGuiKey_W; }

        void options(ToolContext &) override
        {
            ImGui::SetNextItemWidth(110.0f);
            ImGui::SliderFloat("Threshold", &m_threshold, 0.0f, 50.0f, "%.1f");
            ImGui::SetItemTooltip("Largest difference that still joins the region, as in the Seeded tab");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(150.0f);
            ImGui::Combo("##compare", &m_compare, compareNames, IM_COUNTOF(compareNames));
            ImGui::SetItemTooltip("What a neighbour pixel is compared to before it joins");
            ImGui::SameLine();
            ImGui::Checkbox("Limit to selection", &m_limit);
            ImGui::SetItemTooltip("Grow only inside the current selection. Off, Shift+click can add regions outside it");
            m_mode.draw();
        }

        void press(ToolContext &ctx, const ToolEvent &event) override
        {
            ctx.app.magicWand(pixelAt(event.pos), m_threshold, m_compare, m_mode.at(event), m_limit);
        }

    private:
        float m_threshold = 15.0f;
        int m_compare = 0;
        bool m_limit = false;
        ModeChoice m_mode;
    };

    class SeedTool : public Tool
    {
    public:
        const char *name() const override { return "Seeds"; }
        const char *icon() const override { return ICON_CI_LOCATION; }
        const char *help() const override { return "Seeds (S): click to place a seed, Alt+click to remove one. Seeded growing in Placed mode starts from them"; }
        ImGuiKey key() const override { return ImGuiKey_S; }

        void options(ToolContext &ctx) override
        {
            doc::Document &doc = ctx.app.document();
            ImGui::Text("%zu seeds", doc.seeds().size());
            ImGui::SameLine();
            ImGui::BeginDisabled(doc.seeds().empty());
            if (ImGui::Button("Clear"))
                doc::clearSeeds(doc);
            ImGui::EndDisabled();
        }

        void press(ToolContext &ctx, const ToolEvent &event) override
        {
            doc::Document &doc = ctx.app.document();
            if (event.alt)
                doc::removeSeedNear(doc, event.pos, 8.0f / event.zoom); // 8 screen pixels
            else
                doc::addSeed(doc, pixelAt(event.pos));
        }
    };

    std::vector<std::unique_ptr<Tool>> makeTools()
    {
        std::vector<std::unique_ptr<Tool>> tools;
        tools.push_back(makeHandTool());
        tools.push_back(makeBrushTool());
        tools.push_back(makeEraserTool());
        tools.push_back(makePencilTool());
        tools.push_back(makePickerTool());
        tools.push_back(makeBucketTool());
        tools.push_back(std::make_unique<SelectTool>());
        tools.push_back(std::make_unique<WandTool>());
        tools.push_back(std::make_unique<SeedTool>());
        return tools;
    }
}
