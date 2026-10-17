#include "SelectionOverlay.h"

#include "imgui.h"

#include <glad/gl.h>

namespace ui
{
    namespace
    {
        struct OverlayData
        {
            gpu::Renderer *renderer;
            const gpu::Texture *mask;
            gpu::SelectionOverlay where;
        };

        void drawOverlay(const ImDrawList *, const ImDrawCmd *cmd)
        {
            const OverlayData &d = *static_cast<const OverlayData *>(cmd->UserCallbackData);
            if (d.mask->empty())
                return;
            // The backend scissors draws, not callbacks
            GLint viewport[4] = {};
            glGetIntegerv(GL_VIEWPORT, viewport);
            const float scale = float(viewport[2]) / d.where.viewportSize.x;
            const ImVec4 clip = cmd->ClipRect;
            const core::Vec2f pos = d.where.viewportPos;
            glEnable(GL_SCISSOR_TEST);
            glScissor(int((clip.x - pos.x) * scale), int(float(viewport[3]) - (clip.w - pos.y) * scale),
                      int((clip.z - clip.x) * scale), int((clip.w - clip.y) * scale));
            d.renderer->drawSelectionOverlay(*d.mask, d.where);
        }
    }

    void drawSelectionOverlay(gpu::Renderer &renderer, const gpu::Texture &mask, const CanvasView &view, const CanvasInput &canvas)
    {
        const ImGuiViewport *viewport = ImGui::GetWindowViewport();
        OverlayData data{&renderer, &mask, {}};
        data.where.viewportPos = {viewport->Pos.x, viewport->Pos.y};
        data.where.viewportSize = {viewport->Size.x, viewport->Size.y};
        data.where.imageOrigin = view.toScreen({0, 0}, canvas.origin);
        data.where.zoom = view.zoom;
        data.where.time = float(ImGui::GetTime());

        // Without a reset the backend draws on with this program bound
        const ImDrawCallback reset = ImGui::GetPlatformIO().DrawCallback_ResetRenderState;
        if (!reset)
            return;
        ImDrawList *dl = ImGui::GetWindowDrawList();
        const core::Vec2f end = canvas.origin + canvas.size;
        dl->PushClipRect({canvas.origin.x, canvas.origin.y}, {end.x, end.y}, true);
        dl->AddCallback(drawOverlay, &data, sizeof(data));
        dl->AddCallback(reset, nullptr);
        dl->PopClipRect();
    }
}
