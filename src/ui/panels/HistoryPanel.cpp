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

        const doc::History &history = app.document().history();
        ImGui::BeginDisabled(!history.canUndo());
        if (ImGui::Button(ICON_CI_DISCARD " Undo"))
            app.undo();
        ImGui::EndDisabled();
        ImGui::SameLine();
        ImGui::BeginDisabled(!history.canRedo());
        if (ImGui::Button(ICON_CI_REDO " Redo"))
            app.redo();
        ImGui::EndDisabled();
        ImGui::SameLine();
        ImGui::TextDisabled("%.1f of %.0f MB", double(history.bytes()) / (1 << 20), double(history.budget()) / (1 << 20));
        ImGui::SetItemTooltip("Memory the steps keep; past the budget the oldest are dropped");
        ImGui::Separator();

        if (ImGui::BeginChild("##steps"))
        {
            // Follow the current step when it moves
            const bool moved = history.size() != m_seenSize || history.cursor() != m_seenCursor;
            m_seenSize = history.size();
            m_seenCursor = history.cursor();

            const std::string first = history.dropped() > 0 ? "Oldest kept state (" + std::to_string(history.dropped()) + " steps dropped)"
                                                      : "Open " + std::filesystem::path(app.imagePath()).filename().string();
            if (ImGui::Selectable(first.c_str(), history.cursor() == 0))
                app.jumpTo(0);
            if (moved && history.cursor() == 0)
                ImGui::SetScrollHereY();
            for (std::size_t i = 0; i < history.size(); ++i)
            {
                ImGui::PushID(int(i));
                const bool undone = i >= history.cursor();
                if (undone)
                    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
                if (ImGui::Selectable(history.step(i).label().c_str(), history.cursor() == i + 1))
                    app.jumpTo(i + 1);
                if (undone)
                    ImGui::PopStyleColor();
                if (moved && history.cursor() == i + 1)
                    ImGui::SetScrollHereY();
                ImGui::PopID();
            }
        }
        ImGui::EndChild();
        ImGui::End();
    }
}
