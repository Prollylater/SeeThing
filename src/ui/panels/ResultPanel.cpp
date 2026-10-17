#include "ResultPanel.h"

#include "app/App.h"
#include "ui/CanvasWidget.h"
#include "ui/Shell.h"

#include "imgui.h"

namespace ui
{
    void ResultPanel::draw(App &app, Shell &shell)
    {
        OpenGLEngine &gpu = app.gpu();
        if (!gpu.hasResult())
            return;
        bool open = true;
        ImGui::SetNextWindowSize(ImVec2(700, 500), ImGuiCond_FirstUseEver);
        if (ImGui::Begin(name, &open, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
        {
            if (ImGui::Button("Save As..."))
                shell.saveResultDialog();
            ImGui::SameLine();
            if (ImGui::Button("Copy to Image"))
            {
                if (gpu.transferResultToMain())
                    gpu.clearResult();
                else
                    app.log().error("The result and the image differ in size; nothing copied");
            }
            ImGui::SetItemTooltip("Replaces the displayed image with the result. Algorithms still read the opened file (P5)");
            ImGui::SameLine();
            if (ImGui::Button("Discard"))
                gpu.clearResult();

            if (gpu.hasResult())
                canvasWidget("result", gpu.resultTexture(), m_view);
        }
        ImGui::End();
        // Closing the window drops the result, like Discard
        if (!open)
            gpu.clearResult();
    }
}
