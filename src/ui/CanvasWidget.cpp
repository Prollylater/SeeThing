#include "CanvasWidget.h"

#include "imgui.h"

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace ui
{
    namespace
    {
        core::Vec2f vec(ImVec2 v) { return {v.x, v.y}; }
        ImVec2 im(core::Vec2f v) { return {v.x, v.y}; }

        void drawCheckerboard(ImDrawList *dl, core::Vec2f lo, core::Vec2f hi)
        {
            constexpr float cell = 16.0f;
            const ImU32 light = IM_COL32(204, 204, 204, 255), dark = IM_COL32(153, 153, 153, 255);
            dl->AddRectFilled(im(lo), im(hi), light);
            for (float y = lo.y; y < hi.y; y += cell)
                for (float x = lo.x + (int(std::floor((y - lo.y) / cell)) % 2 ? 0.0f : cell); x < hi.x; x += 2 * cell)
                    dl->AddRectFilled({x, y}, {std::min(x + cell, hi.x), std::min(y + cell, hi.y)}, dark);
        }

        void drawPixelGrid(ImDrawList *dl, const CanvasView &view, core::Vec2f origin, core::Vec2f lo, core::Vec2f hi, core::Vec2f imageSize)
        {
            const ImU32 color = IM_COL32(128, 128, 128, 90);
            const core::Vec2f first = view.toImage(lo, origin), last = view.toImage(hi, origin);
            for (int x = std::max(0, int(std::ceil(first.x))); x <= std::min(int(imageSize.x), int(last.x)); ++x)
            {
                const float sx = view.toScreen({float(x), 0}, origin).x;
                dl->AddLine({sx, lo.y}, {sx, hi.y}, color);
            }
            for (int y = std::max(0, int(std::ceil(first.y))); y <= std::min(int(imageSize.y), int(last.y)); ++y)
            {
                const float sy = view.toScreen({0, float(y)}, origin).y;
                dl->AddLine({lo.x, sy}, {hi.x, sy}, color);
            }
        }
    }

    CanvasInput canvasWidget(const char *id, const gpu::Texture &tex, CanvasView &view)
    {
        CanvasInput input;
        ImGui::PushID(id);
        const core::Vec2f imageSize{float(tex.width()), float(tex.height())};

        const bool fitClicked = ImGui::Button("Fit");
        ImGui::SameLine();
        const bool actualClicked = ImGui::Button("100%");
        ImGui::SameLine();
        ImGui::Text("%.0f%%", view.zoom * 100.0f);

        const core::Vec2f origin = vec(ImGui::GetCursorScreenPos());
        core::Vec2f size = vec(ImGui::GetContentRegionAvail());
        size.y -= ImGui::GetTextLineHeightWithSpacing();
        size = {std::max(size.x, 32.0f), std::max(size.y, 32.0f)};

        if (fitClicked || view.request == CanvasView::Request::Fit || !(view.fittedSize == imageSize))
            view.fit(imageSize, size);
        if (actualClicked || view.request == CanvasView::Request::ActualSize)
            view.actualSize(imageSize, size);
        view.request = CanvasView::Request::None;

        ImGui::InvisibleButton("##canvas", im(size),
                               ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight | ImGuiButtonFlags_MouseButtonMiddle);
        const ImGuiID itemId = ImGui::GetItemID();
        const bool hovered = ImGui::IsItemHovered();
        const bool active = ImGui::IsItemActive();
        const ImGuiIO &io = ImGui::GetIO();
        const bool spaceHeld = ImGui::IsKeyDown(ImGuiKey_Space);

        if (hovered && io.MouseWheel != 0.0f)
            view.zoomAround(vec(io.MousePos), origin, CanvasView::stepZoom(view.zoom, io.MouseWheel));
        if (active && (ImGui::IsMouseDragging(ImGuiMouseButton_Right, 0.0f) || ImGui::IsMouseDragging(ImGuiMouseButton_Middle, 0.0f) ||
                       (spaceHeld && ImGui::IsMouseDown(ImGuiMouseButton_Left))))
            view.pan += vec(io.MouseDelta);
        // Whole screen pixels keep nearest sampling crisp
        view.pan = {std::round(view.pan.x), std::round(view.pan.y)};

        ImGuiStorage *storage = ImGui::GetStateStorage();
        bool painting = storage->GetBool(itemId, false);
        if (ImGui::IsItemActivated() && io.MouseClicked[ImGuiMouseButton_Left] && !spaceHeld)
        {
            painting = true;
            input.strokeStarted = true;
        }
        else if (painting && (!active || !ImGui::IsMouseDown(ImGuiMouseButton_Left)))
        {
            painting = false;
            input.strokeEnded = true;
        }
        storage->SetBool(itemId, painting);

        input.hovered = hovered;
        input.painting = painting;
        input.mouse = view.toImage(vec(io.MousePos), origin);

        const core::Vec2f viewEnd = origin + size;
        ImDrawList *dl = ImGui::GetWindowDrawList();
        dl->PushClipRect(im(origin), im(viewEnd), true);
        dl->AddRectFilled(im(origin), im(viewEnd), IM_COL32(40, 40, 40, 255));
        if (!tex.empty())
        {
            const core::Vec2f p0 = view.toScreen({0, 0}, origin), p1 = view.toScreen(imageSize, origin);
            const core::Vec2f lo{std::max(p0.x, origin.x), std::max(p0.y, origin.y)};
            const core::Vec2f hi{std::min(p1.x, viewEnd.x), std::min(p1.y, viewEnd.y)};
            if (lo.x < hi.x && lo.y < hi.y)
            {
                if (tex.channels() == 2 || tex.channels() == 4)
                    drawCheckerboard(dl, lo, hi);

                // The backend's own sampler is linear: nearest when enlarged, linear when reduced
                const ImGuiPlatformIO &pio = ImGui::GetPlatformIO();
                ImDrawCallback sampler = view.zoom >= 1.0f ? pio.DrawCallback_SetSamplerNearest : pio.DrawCallback_SetSamplerLinear;
                if (sampler)
                    dl->AddCallback(sampler, nullptr);
                dl->AddImage((ImTextureID)(std::intptr_t)tex.id(), im(p0), im(p1));
                if (pio.DrawCallback_ResetRenderState)
                    dl->AddCallback(pio.DrawCallback_ResetRenderState, nullptr);

                if (view.zoom >= 8.0f)
                    drawPixelGrid(dl, view, origin, lo, hi, imageSize);
            }
            dl->AddRect(im(p0 - core::Vec2f{1, 1}), im(p1 + core::Vec2f{1, 1}), IM_COL32(90, 90, 90, 255));
        }
        dl->PopClipRect();

        const int px = int(std::floor(input.mouse.x)), py = int(std::floor(input.mouse.y));
        if (hovered && px >= 0 && py >= 0 && px < tex.width() && py < tex.height())
            ImGui::Text("%d x %d    %d, %d", tex.width(), tex.height(), px, py);
        else
            ImGui::Text("%d x %d", tex.width(), tex.height());

        ImGui::PopID();
        return input;
    }
}
