#include "LogPanel.h"

#include "imgui.h"

#include <ctime>
#include <string>

namespace ui
{
    namespace
    {
        std::string clockTime(std::chrono::system_clock::time_point t)
        {
            const std::time_t seconds = std::chrono::system_clock::to_time_t(t);
            std::tm local{};
            localtime_r(&seconds, &local);
            char text[16];
            std::strftime(text, sizeof text, "%H:%M:%S", &local);
            return text;
        }
    }

    void LogPanel::draw(core::Log &log, bool &open)
    {
        if (!open)
            return;
        ImGui::SetNextWindowSize(ImVec2(700, 200), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin(name, &open))
        {
            ImGui::End();
            return;
        }

        const std::size_t revision = log.revision();
        if (revision != m_revision)
        {
            m_entries = log.snapshot();
            m_revision = revision;
            m_scrollToEnd = true;
        }

        auto visible = [&](const core::LogEntry &e)
        {
            switch (e.level)
            {
            case core::LogLevel::Info: return m_showInfo;
            case core::LogLevel::Warning: return m_showWarnings;
            case core::LogLevel::Error: return m_showErrors;
            }
            return true;
        };

        if (ImGui::Button("Clear"))
            log.clear();
        ImGui::SameLine();
        if (ImGui::Button("Copy"))
        {
            std::string text;
            for (const core::LogEntry &e : m_entries)
                if (visible(e))
                    text += clockTime(e.time) + "  " + e.text + "\n";
            ImGui::SetClipboardText(text.c_str());
        }
        ImGui::SameLine();
        ImGui::Checkbox("Info", &m_showInfo);
        ImGui::SameLine();
        ImGui::Checkbox("Warnings", &m_showWarnings);
        ImGui::SameLine();
        ImGui::Checkbox("Errors", &m_showErrors);
        ImGui::Separator();

        if (ImGui::BeginChild("##lines", ImVec2(0, 0), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar))
        {
            // Follow new lines only when already at the bottom, so reading older ones isn't interrupted
            const bool atEnd = ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 1.0f;
            const ImVec4 warnColor(0.95f, 0.75f, 0.30f, 1.0f), errorColor(1.0f, 0.45f, 0.40f, 1.0f);
            for (const core::LogEntry &e : m_entries)
            {
                if (!visible(e))
                    continue;
                ImGui::TextDisabled("%s", clockTime(e.time).c_str());
                ImGui::SameLine();
                if (e.level == core::LogLevel::Info)
                    ImGui::TextUnformatted(e.text.c_str());
                else
                    ImGui::TextColored(e.level == core::LogLevel::Error ? errorColor : warnColor, "%s", e.text.c_str());
            }
            if (m_scrollToEnd && atEnd)
                ImGui::SetScrollHereY(1.0f);
            m_scrollToEnd = false;
        }
        ImGui::EndChild();
        ImGui::End();
    }
}
