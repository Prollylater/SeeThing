#ifndef UI_LAYERSPANEL_H
#define UI_LAYERSPANEL_H

#include "doc/Layer.h"

class App;

namespace ui
{
    // The layer stack, top first, and the active layer's properties
    class LayersPanel
    {
    public:
        static constexpr const char *name = "Layers";

        void draw(App &app, bool &open);

    private:
        void drawProperties(App &app);
        void drawList(App &app);
        void drawRenamePopup(App &app);

        // Where the opacity drag started, recorded as one step on release
        doc::LayerProps m_dragStart;
        doc::LayerId m_renaming = 0;
        char m_renameBuffer[128] = {};
    };
}

#endif
