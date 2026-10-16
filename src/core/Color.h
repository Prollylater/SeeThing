#ifndef CORE_COLOR_H
#define CORE_COLOR_H

#include "Geometry.h"
#include "Image.h"

#include <cstdint>

// The only place that knows the colour formulas and the white points.
// Sources: http://www.brucelindbloom.com/ (equations, reference values) and https://www.easyrgb.com/en/math.php
namespace core
{
    // CIE 1931 2° observer; same set and order as XYZRefHandle in Utils/Mat.h
    enum class WhitePoint
    {
        A,
        B,
        C,
        D50,
        D55,
        D65,
        D75,
        E,
        F1,
    };

    // XYZ of the white point, scaled so Y = 1
    Vec3f whitePointXYZ(WhitePoint wp);
    const char *whitePointName(WhitePoint wp);

    float srgbToLinear(float v);
    float linearToSrgb(float v);

    // sRGB primaries are defined for D65. Another white point only changes the Lab normalisation, with no
    // chromatic adaptation, as the existing SLIC panel does.
    Vec3f srgbToLab(Vec3f srgb01, WhitePoint wp = WhitePoint::D65);
    Vec3f labToSrgb(Vec3f lab, WhitePoint wp = WhitePoint::D65);

    // Converters return a new image; none of them work in place.
    // Inputs with 1-2 channels are gray (+alpha), 3-4 are RGB (+alpha); alpha is dropped unless kept by the output.
    Image<std::uint8_t> toGray8(ImageView<const std::uint8_t> src);
    Image<std::uint8_t> toRGB8(ImageView<const std::uint8_t> src);
    Image<std::uint8_t> toRGBA8(ImageView<const std::uint8_t> src);
    Image<float> toLab(ImageView<const std::uint8_t> src, WhitePoint wp = WhitePoint::D65);
    Image<std::uint8_t> labToRGB8(ImageView<const float> lab, WhitePoint wp = WhitePoint::D65);
}

#endif
