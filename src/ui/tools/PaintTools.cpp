#include "Tool.h"
#include "Tools.h"

#include "IconsCodicons.h"
#include "app/App.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace ui
{
    namespace
    {
        core::Vec2i pixelAt(core::Vec2f p) { return {int(std::floor(p.x)), int(std::floor(p.y))}; }

        void colorOption(ToolShared &shared)
        {
            ImGui::ColorEdit3("##color", &shared.color.x, ImGuiColorEditFlags_NoInputs);
            ImGui::SetItemTooltip("Foreground colour");
        }

        void regionOptions(float &threshold, int &compare)
        {
            ImGui::SetNextItemWidth(110.0f);
            ImGui::SliderFloat("Threshold", &threshold, 0.0f, 50.0f, "%.1f");
            ImGui::SetItemTooltip("Largest difference that still joins the region, as in the Seeded tab");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(150.0f);
            ImGui::Combo("##compare", &compare, compareNames, IM_COUNTOF(compareNames));
            ImGui::SetItemTooltip("What a neighbour pixel is compared to before it joins");
        }
    }

    class HandTool : public Tool
    {
    public:
        const char *name() const override { return "Hand"; }
        const char *icon() const override { return ICON_CI_MOVE; }
        const char *help() const override { return "Hand (H): left drag pans"; }
        ImGuiKey key() const override { return ImGuiKey_H; }
        bool leftDragPans() const override { return true; }
    };

    // Squares between two frames' positions, so a fast drag leaves no gaps. The stamp brush replaces it.
    class PencilTool : public Tool
    {
    public:
        const char *name() const override { return "Pencil"; }
        const char *icon() const override { return ICON_CI_EDIT; }
        const char *help() const override { return "Pencil (P): hard square pixels on the active layer"; }
        ImGuiKey key() const override { return ImGuiKey_P; }

        void options(ToolContext &ctx) override
        {
            colorOption(ctx.shared);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(120.0f);
            ImGui::SliderInt("Size", &m_size, 1, 32, "%d px");
        }

        void press(ToolContext &ctx, const ToolEvent &e) override
        {
            m_active = ctx.app.beginStroke();
            m_last = snapped(e.pos);
            if (m_active)
                paint(ctx, std::vector<core::Vec2f>{m_last});
        }

        void drag(ToolContext &ctx, const ToolEvent &e) override
        {
            if (!m_active)
                return;
            const core::Vec2f pos = snapped(e.pos);
            const core::Vec2f delta = pos - m_last;
            const int steps = int(std::max(std::fabs(delta.x), std::fabs(delta.y)));
            std::vector<core::Vec2f> points;
            for (int i = 1; i <= steps; ++i)
            {
                const core::Vec2f p = m_last + delta * (float(i) / steps);
                points.push_back({std::round(p.x), std::round(p.y)});
            }
            m_last = pos;
            paint(ctx, points);
        }

        void release(ToolContext &ctx, const ToolEvent &) override { cancel(ctx); }
        void cancel(ToolContext &ctx) override
        {
            if (m_active)
                ctx.app.endStroke();
            m_active = false;
        }

        void overlay(ImDrawList *dl, const CanvasView &view, const CanvasInput &canvas) const override
        {
            if (!canvas.hovered)
                return;
            const float half = float(m_size) * 0.5f;
            const core::Vec2f centre{std::floor(canvas.mouse.x) + 0.5f, std::floor(canvas.mouse.y) + 0.5f};
            const core::Vec2f a = view.toScreen(centre - core::Vec2f{half, half}, canvas.origin);
            const core::Vec2f b = view.toScreen(centre + core::Vec2f{half, half}, canvas.origin);
            // Dark then light, visible on any image
            dl->AddRect({a.x - 1, a.y - 1}, {b.x + 1, b.y + 1}, IM_COL32(0, 0, 0, 160));
            dl->AddRect({a.x, a.y}, {b.x, b.y}, IM_COL32(255, 255, 255, 200));
        }

    private:
        static core::Vec2f snapped(core::Vec2f p) { return {std::floor(p.x), std::floor(p.y)}; }
        void paint(ToolContext &ctx, const std::vector<core::Vec2f> &points)
        {
            const ImVec4 &c = ctx.shared.color;
            ctx.app.strokePoints(points, {c.x, c.y, c.z}, float(m_size));
        }

        int m_size = 2;
        bool m_active = false;
        core::Vec2f m_last;
    };

    class PickerTool : public Tool
    {
    public:
        const char *name() const override { return "Picker"; }
        const char *icon() const override { return ICON_CI_SYMBOL_COLOR; }
        const char *help() const override { return "Colour picker (I): click or drag to take a colour"; }
        ImGuiKey key() const override { return ImGuiKey_I; }

        void options(ToolContext &ctx) override
        {
            colorOption(ctx.shared);
            ImGui::SameLine();
            ImGui::Checkbox("All layers", &m_composite);
            ImGui::SetItemTooltip("On: the colour you see. Off: the active layer's own pixel");
        }

        void press(ToolContext &ctx, const ToolEvent &e) override { pick(ctx, e); }
        void drag(ToolContext &ctx, const ToolEvent &e) override { pick(ctx, e); }

    private:
        void pick(ToolContext &ctx, const ToolEvent &e)
        {
            if (const auto rgba = ctx.app.pickColor(pixelAt(e.pos), m_composite))
                ctx.shared.color = {(*rgba)[0] / 255.0f, (*rgba)[1] / 255.0f, (*rgba)[2] / 255.0f, 1.0f};
        }

        bool m_composite = true;
    };

    class BucketTool : public Tool
    {
    public:
        const char *name() const override { return "Bucket"; }
        const char *icon() const override { return ICON_CI_PAINTCAN; }
        const char *help() const override { return "Bucket (G): fills the region grown from the click, on the active layer"; }
        ImGuiKey key() const override { return ImGuiKey_G; }

        void options(ToolContext &ctx) override
        {
            colorOption(ctx.shared);
            ImGui::SameLine();
            regionOptions(m_threshold, m_compare);
        }

        void press(ToolContext &ctx, const ToolEvent &e) override
        {
            const ImVec4 &c = ctx.shared.color;
            const auto to8 = [](float v) { return std::uint8_t(std::lround(std::clamp(v, 0.0f, 1.0f) * 255.0f)); };
            ctx.app.bucketFill(pixelAt(e.pos), m_threshold, m_compare, {to8(c.x), to8(c.y), to8(c.z), 255});
        }

    private:
        float m_threshold = 15.0f;
        int m_compare = 0;
    };

    std::unique_ptr<Tool> makeHandTool() { return std::make_unique<HandTool>(); }
    std::unique_ptr<Tool> makePencilTool() { return std::make_unique<PencilTool>(); }
    std::unique_ptr<Tool> makePickerTool() { return std::make_unique<PickerTool>(); }
    std::unique_ptr<Tool> makeBucketTool() { return std::make_unique<BucketTool>(); }
}
