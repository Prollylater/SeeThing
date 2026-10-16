#ifndef CORE_PIXELFORMAT_H
#define CORE_PIXELFORMAT_H

#include <cstdint>
#include <type_traits>

namespace core
{
    enum class PixelFormat
    {
        Unknown,
        R8,
        RG8,
        RGB8,
        RGBA8,
        R32F,
        RGB32F,
        RGBA32F,
        R32I,
    };

    // What the channel values mean; Labels and Mask tag single-channel results
    enum class ColorSpace
    {
        sRGB,
        Linear,
        Lab,
        Gray,
        Labels,
        Mask,
    };

    template <class T>
    constexpr PixelFormat pixelFormatOf(int channels)
    {
        if constexpr (std::is_same_v<T, std::uint8_t>)
        {
            switch (channels)
            {
            case 1: return PixelFormat::R8;
            case 2: return PixelFormat::RG8;
            case 3: return PixelFormat::RGB8;
            case 4: return PixelFormat::RGBA8;
            }
        }
        else if constexpr (std::is_same_v<T, float>)
        {
            switch (channels)
            {
            case 1: return PixelFormat::R32F;
            case 3: return PixelFormat::RGB32F;
            case 4: return PixelFormat::RGBA32F;
            }
        }
        else if constexpr (std::is_same_v<T, std::int32_t>)
        {
            if (channels == 1)
                return PixelFormat::R32I;
        }
        return PixelFormat::Unknown;
    }
}

#endif
