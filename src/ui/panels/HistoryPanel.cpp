#include "HistoryPanel.h"

#include "IconsCodicons.h"
#include "app/App.h"

#include "imgui.h"

#include <filesystem>
#include <string>

namespace ui
{
    void HistoryPanel::draw(App &app, bool &open)
    {
        if (!open)
            return;
        if (!ImGui::Begin(name, &open))
        {
            ImGui::End();
            return;
        }
        if (!app.hasImage())
        {
            ImGui::TextDisabled("No image.");
            ImGui::End();
            return;
        }

        const doc::History &h = app.document().history();
        ImGui::BeginDisabled(!h.canUndo());
        if (ImGui::Button(ICON_CI_DISCARD " Undo"))
            app.undo();
        ImGui::EndDisabled();
        ImGui::SameLine();
        ImGui::BeginDisabled(!h.canRedo());
        if (ImGui::Button(ICON_CI_REDO " Redo"))
            app.redo();
        ImGui::EndDisabled();
        ImGui::SameLine();
        ImGui::TextDisabled("%.1f of %.0f MB", double(h.bytes()) / (1 << 20), double(h.budget()) / (1 << 20));
        ImGui::SetItemTooltip("Memory the steps keep; past the budget the oldest are dropped");
        ImGui::Separator();

        if (ImGui::BeginChild("##steps"))
        {
            // Follow the current step when it moves
            const bool moved = h.size() != m_seenSize || h.cursor() != m_seenCursor;
            m_seenSize = h.size();
            m_seenCursor = h.cursor();

            const std::string first = h.dropped() > 0 ? "Oldest kept state (" + std::to_string(h.dropped()) + " steps dropped)"
                                                      : "Open " + std::filesystem::path(app.imagePath()).filename().string();
            if (ImGui::Selectable(first.c_str(), h.cursor() == 0))
                app.jumpTo(0);
            if (moved && h.cursor() == 0)
                ImGui::SetScrollHereY();
            for (std::size_t i = 0; i < h.size(); ++i)
            {
                ImGui::PushID(int(i));
                const bool undone = i >= h.cursor();
                if (undone)
                    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
                if (ImGui::Selectable(h.step(i).label().c_str(), h.cursor() == i + 1))
                    app.jumpTo(i + 1);
                if (undone)
                    ImGui::PopStyleColor();
                if (moved && h.cursor() == i + 1)
                    ImGui::SetScrollHereY();
                ImGui::PopID();
            }
        }
        ImGui::EndChild();
        ImGui::End();
    }
}
