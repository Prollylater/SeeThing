#ifndef UI_HISTORYPANEL_H
#define UI_HISTORYPANEL_H

#include <cstddef>

class App;

namespace ui
{
    class HistoryPanel
    {
    public:
        static constexpr const char *name = "History";

        void draw(App &app, bool &open);

    private:
        std::size_t m_seenSize = 0, m_seenCursor = 0;
    };
}

#endif
