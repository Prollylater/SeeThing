#ifndef CORE_LOG_H
#define CORE_LOG_H

#include <chrono>
#include <cstddef>
#include <deque>
#include <format>
#include <mutex>
#include <string>
#include <vector>

namespace core
{
    enum class LogLevel
    {
        Info,
        Warning,
        Error,
    };

    struct LogEntry
    {
        LogLevel level;
        std::chrono::system_clock::time_point time;
        std::string text;
    };

    // Messages for the Log panel, also echoed to stderr. Safe to call from any thread.
    class Log
    {
    public:
        explicit Log(std::size_t capacity = 2000) : m_capacity(capacity) {}

        void add(LogLevel level, std::string text);

        template <class... Args>
        void info(std::format_string<Args...> fmt, Args &&...args) { add(LogLevel::Info, std::format(fmt, std::forward<Args>(args)...)); }
        template <class... Args>
        void warn(std::format_string<Args...> fmt, Args &&...args) { add(LogLevel::Warning, std::format(fmt, std::forward<Args>(args)...)); }
        template <class... Args>
        void error(std::format_string<Args...> fmt, Args &&...args) { add(LogLevel::Error, std::format(fmt, std::forward<Args>(args)...)); }

        std::vector<LogEntry> snapshot() const;
        // Grows on every add; lets a reader skip copying when nothing changed
        std::size_t revision() const;
        void clear();
        void setEcho(bool echo) { m_echo = echo; }

    private:
        mutable std::mutex m_mutex;
        std::deque<LogEntry> m_entries;
        std::size_t m_capacity;
        std::size_t m_revision = 0;
        bool m_echo = true;
    };
}

#endif
