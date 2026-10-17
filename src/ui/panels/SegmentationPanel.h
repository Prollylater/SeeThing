#ifndef UI_SEGMENTATIONPANEL_H
#define UI_SEGMENTATIONPANEL_H

class App;

namespace ui
{
    // SLIC and seeded region growing parameters; Run sends them to the App
    class SegmentationPanel
    {
    public:
        static constexpr const char *name = "Segmentation";

        void draw(App &app, bool &open);

    private:
        void drawSlic(App &app);
        void drawSeeded(App &app);

        struct Slic
        {
            int k = 100;
            int compactness = 20;
            bool scaleXyz = false;
            int xyzScale = 1;
            int illuminant = 5; // D65_2 in Mat.h's XYZRefHandle
            int coloration = 0;
        };
        struct Seeded
        {
            int seeds = 100;
            int source = 0; // random, grid, placed
            int compare = 0;
            bool adaptive = false;
            float adaptPercent = 50.0f;
            float threshold = 15.0f;
            int coloration = 0;
        };

        Slic m_slic;
        Seeded m_seeded;
    };
}

#endif
