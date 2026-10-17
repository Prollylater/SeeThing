#include "Log.h"

#include <cstdio>

namespace core
{
    void Log::add(LogLevel level, std::string text)
    {
        if (m_echo)
        {
            const char *tag = level == LogLevel::Error ? "error: " : level == LogLevel::Warning ? "warning: " : "";
            std::fprintf(stderr, "%s%s\n", tag, text.c_str());
        }
        std::lock_guard lock(m_mutex);
        m_entries.push_back({level, std::chrono::system_clock::now(), std::move(text)});
        if (m_entries.size() > m_capacity)
            m_entries.pop_front();
        ++m_revision;
    }

    std::vector<LogEntry> Log::snapshot() const
    {
        std::lock_guard lock(m_mutex);
        return {m_entries.begin(), m_entries.end()};
    }

    std::size_t Log::revision() const
    {
        std::lock_guard lock(m_mutex);
        return m_revision;
    }

    void Log::clear()
    {
        std::lock_guard lock(m_mutex);
        m_entries.clear();
        ++m_revision;
    }
}
