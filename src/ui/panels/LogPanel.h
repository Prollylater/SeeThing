#ifndef UI_LOGPANEL_H
#define UI_LOGPANEL_H

#include "core/Log.h"

#include <cstddef>
#include <vector>

namespace ui
{
    // What used to go to std::cout and std::cerr
    class LogPanel
    {
    public:
        static constexpr const char *name = "Log";

        void draw(core::Log &log, bool &open);

    private:
        std::vector<core::LogEntry> m_entries;
        std::size_t m_revision = std::size_t(-1);
        bool m_showInfo = true, m_showWarnings = true, m_showErrors = true;
        bool m_scrollToEnd = false;
    };
}

#endif
