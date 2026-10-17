#include "ResultPanel.h"

#include "app/App.h"
#include "ui/CanvasWidget.h"
#include "ui/Shell.h"

#include "imgui.h"

namespace ui
{
    void ResultPanel::draw(App &app, Shell &shell)
    {
        if (!app.hasResult())
            return;
        bool open = true;
        ImGui::SetNextWindowSize(ImVec2(700, 500), ImGuiCond_FirstUseEver);
        if (ImGui::Begin(name, &open, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
        {
            ImGui::TextUnformatted(app.resultLabel().c_str());
            ImGui::SameLine();
            if (ImGui::Button("Save As..."))
                shell.saveResultDialog();
            ImGui::SameLine();
            const bool hasImage = app.hasImage();
            ImGui::BeginDisabled(!hasImage);
            if (ImGui::Button("Add as Layer"))
                app.addResultAsLayer();
            ImGui::SetItemTooltip("A new Result layer above the one the algorithm read");
            ImGui::SameLine();
            if (ImGui::Button("Copy to Layer"))
                app.copyResultToLayer();
            ImGui::SetItemTooltip("Replaces the pixels of the active layer");
            ImGui::EndDisabled();
            ImGui::SameLine();
            if (ImGui::Button("Discard"))
                app.discardResult();

            if (app.hasResult())
                canvasWidget("result", app.gpu().resultTexture(), m_view);
        }
        ImGui::End();
        // Closing the window drops the result, like Discard
        if (!open)
            app.discardResult();
    }
}
