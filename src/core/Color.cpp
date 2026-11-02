#include "Color.h"

#include <cmath>
#include <stdexcept>

namespace core
{
    namespace
    {
        struct WhitePointEntry
        {
            Vec3f xyz;
            const char *name;
        };

        constexpr WhitePointEntry whitePoints[] = {
            {{1.09850f, 1.0f, 0.35585f}, "A (incandescent)"},
            {{0.99092f, 1.0f, 0.85313f}, "B (old direct sunlight)"},
            {{0.98074f, 1.0f, 1.18232f}, "C (old daylight)"},
            {{0.96422f, 1.0f, 0.82521f}, "D50 (ICC profile PCS)"},
            {{0.95682f, 1.0f, 0.92149f}, "D55 (mid-morning daylight)"},
            {{0.95047f, 1.0f, 1.08883f}, "D65 (daylight, sRGB)"},
            {{0.94972f, 1.0f, 1.22638f}, "D75 (north sky daylight)"},
            {{1.00000f, 1.0f, 1.00000f}, "E (equal energy)"},
            {{0.92834f, 1.0f, 1.03665f}, "F1 (daylight fluorescent)"},
        };

        // sRGB (D65) linear RGB to XYZ, Lindbloom
        const Mat3f rgbToXyz = {{{{0.4124564f, 0.3575761f, 0.1804375f},
                                  {0.2126729f, 0.7151522f, 0.0721750f},
                                  {0.0193339f, 0.1191920f, 0.9503041f}}}};
        const Mat3f xyzToRgb = *rgbToXyz.inverse();

        // Exact CIE constants instead of the rounded 0.008856 and 7.787
        constexpr float labEpsilon = 216.0f / 24389.0f;
        constexpr float labKappa = 24389.0f / 27.0f;

        float labF(float t) { return t > labEpsilon ? std::cbrt(t) : (labKappa * t + 16.0f) / 116.0f; }
        float labFInv(float f)
        {
            float f3 = f * f * f;
            return f3 > labEpsilon ? f3 : (116.0f * f - 16.0f) / labKappa;
        }

        std::uint8_t toByte(float v01)
        {
            float scaled = std::round(v01 * 255.0f);
            return static_cast<std::uint8_t>(scaled < 0.0f ? 0.0f : (scaled > 255.0f ? 255.0f : scaled));
        }

        void requireChannels(int channels)
        {
            if (channels < 1 || channels > 4)
                throw std::invalid_argument("colour converter: expected 1 to 4 channels");
        }

        // Rec. 601 luma on the encoded values, as OpenCV and most viewers do
        std::uint8_t luma(const std::uint8_t *pixel, int channels)
        {
            if (channels < 3)
                return pixel[0];
            return static_cast<std::uint8_t>(std::lround(0.299f * pixel[0] + 0.587f * pixel[1] + 0.114f * pixel[2]));
        }
    }

    Vec3f whitePointXYZ(WhitePoint whitePt) { return whitePoints[static_cast<int>(whitePt)].xyz; }
    const char *whitePointName(WhitePoint whitePt) { return whitePoints[static_cast<int>(whitePt)].name; }

    float srgbToLinear(float encoded) { return encoded <= 0.04045f ? encoded / 12.92f : std::pow((encoded + 0.055f) / 1.055f, 2.4f); }
    float linearToSrgb(float linear) { return linear <= 0.0031308f ? linear * 12.92f : 1.055f * std::pow(linear, 1.0f / 2.4f) - 0.055f; }

    Vec3f srgbToLab(Vec3f srgb01, WhitePoint whitePt)
    {
        Vec3f lin{srgbToLinear(srgb01.x), srgbToLinear(srgb01.y), srgbToLinear(srgb01.z)};
        Vec3f xyz = rgbToXyz * lin;
        Vec3f white = whitePointXYZ(whitePt);
        float fx = labF(xyz.x / white.x), fy = labF(xyz.y / white.y), fz = labF(xyz.z / white.z); // f(X/Xn), f(Y/Yn), f(Z/Zn) of the CIE formula
        return {116.0f * fy - 16.0f, 500.0f * (fx - fy), 200.0f * (fy - fz)};
    }

    Vec3f labToSrgb(Vec3f lab, WhitePoint whitePt)
    {
        float fy = (lab.x + 16.0f) / 116.0f; // f(Y/Yn), then f(X/Xn) and f(Z/Zn)
        float fx = fy + lab.y / 500.0f;
        float fz = fy - lab.z / 200.0f;
        Vec3f white = whitePointXYZ(whitePt);
        Vec3f xyz{labFInv(fx) * white.x, labFInv(fy) * white.y, labFInv(fz) * white.z};
        Vec3f lin = xyzToRgb * xyz;
        return {linearToSrgb(lin.x), linearToSrgb(lin.y), linearToSrgb(lin.z)};
    }

    Image<std::uint8_t> toGray8(ImageView<const std::uint8_t> src)
    {
        requireChannels(src.channels());
        Image<std::uint8_t> out(src.width(), src.height(), 1, ColorSpace::Gray);
        for (int y = 0; y < src.height(); ++y)
            for (int x = 0; x < src.width(); ++x)
                out(x, y) = luma(&src(x, y), src.channels());
        return out;
    }

    Image<std::uint8_t> toRGB8(ImageView<const std::uint8_t> src)
    {
        requireChannels(src.channels());
        Image<std::uint8_t> out(src.width(), src.height(), 3);
        for (int y = 0; y < src.height(); ++y)
            for (int x = 0; x < src.width(); ++x)
                for (int c = 0; c < 3; ++c)
                    out(x, y, c) = src(x, y, src.channels() < 3 ? 0 : c);
        return out;
    }

    Image<std::uint8_t> toRGBA8(ImageView<const std::uint8_t> src)
    {
        requireChannels(src.channels());
        const int channels = src.channels();
        Image<std::uint8_t> out(src.width(), src.height(), 4);
        for (int y = 0; y < src.height(); ++y)
            for (int x = 0; x < src.width(); ++x)
            {
                for (int k = 0; k < 3; ++k)
                    out(x, y, k) = src(x, y, channels < 3 ? 0 : k);
                out(x, y, 3) = (channels == 2 || channels == 4) ? src(x, y, channels - 1) : 255;
            }
        return out;
    }

    Image<float> toLab(ImageView<const std::uint8_t> src, WhitePoint whitePt)
    {
        requireChannels(src.channels());
        const int channels = src.channels();
        Image<float> out(src.width(), src.height(), 3, ColorSpace::Lab);
        for (int y = 0; y < src.height(); ++y)
            for (int x = 0; x < src.width(); ++x)
            {
                const std::uint8_t *pixel = &src(x, y);
                Vec3f rgb = channels < 3 ? Vec3f{pixel[0] / 255.0f, pixel[0] / 255.0f, pixel[0] / 255.0f}
                                  : Vec3f{pixel[0] / 255.0f, pixel[1] / 255.0f, pixel[2] / 255.0f};
                Vec3f lab = srgbToLab(rgb, whitePt);
                for (int k = 0; k < 3; ++k)
                    out(x, y, k) = lab[k];
            }
        return out;
    }

    Image<std::uint8_t> labToRGB8(ImageView<const float> lab, WhitePoint whitePt)
    {
        if (lab.channels() != 3)
            throw std::invalid_argument("labToRGB8: expected 3 channels");
        Image<std::uint8_t> out(lab.width(), lab.height(), 3);
        for (int y = 0; y < lab.height(); ++y)
            for (int x = 0; x < lab.width(); ++x)
            {
                Vec3f rgb = labToSrgb({lab(x, y, 0), lab(x, y, 1), lab(x, y, 2)}, whitePt);
                for (int k = 0; k < 3; ++k)
                    out(x, y, k) = toByte(rgb[k]);
            }
        return out;
    }
}
