#ifndef CORE_BLENDMODE_H
#define CORE_BLENDMODE_H

namespace core
{
    // Separable modes of W3C Compositing and Blending Level 1. The values are what composite.frag switches on.
    enum class BlendMode
    {
        Normal,
        Multiply,
        Screen,
        Overlay,
        Difference,
    };

    inline constexpr BlendMode blendModes[] = {BlendMode::Normal, BlendMode::Multiply, BlendMode::Screen, BlendMode::Overlay, BlendMode::Difference};

    inline const char *blendModeName(BlendMode mode)
    {
        switch (mode)
        {
        case BlendMode::Normal: return "Normal";
        case BlendMode::Multiply: return "Multiply";
        case BlendMode::Screen: return "Screen";
        case BlendMode::Overlay: return "Overlay";
        case BlendMode::Difference: return "Difference";
        }
        return "?";
    }
}

#endif
