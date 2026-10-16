#include <doctest/doctest.h>

#include "core/Color.h"

#include <algorithm>
#include <cmath>
#include <cstdint>

using core::Vec3f;
using core::WhitePoint;

namespace
{
    void checkLab(Vec3f got, Vec3f want, double tolerance = 0.01)
    {
        CHECK(std::abs(got.x - want.x) < tolerance);
        CHECK(std::abs(got.y - want.y) < tolerance);
        CHECK(std::abs(got.z - want.z) < tolerance);
    }
}

// Reference values: Bruce Lindbloom's colour calculator, sRGB with D65, 2° observer
TEST_CASE("Color: sRGB to Lab matches reference values")
{
    checkLab(core::srgbToLab({1, 1, 1}), {100.0f, 0.0f, 0.0f});
    checkLab(core::srgbToLab({0, 0, 0}), {0.0f, 0.0f, 0.0f});
    checkLab(core::srgbToLab({1, 0, 0}), {53.2408f, 80.0925f, 67.2032f});
    checkLab(core::srgbToLab({0, 1, 0}), {87.7347f, -86.1827f, 83.1793f});
    checkLab(core::srgbToLab({0, 0, 1}), {32.2970f, 79.1875f, -107.8602f});
    checkLab(core::srgbToLab({128 / 255.0f, 128 / 255.0f, 128 / 255.0f}), {53.5850f, 0.0f, 0.0f});
}

TEST_CASE("Color: Lab to sRGB inverts sRGB to Lab")
{
    for (Vec3f rgb : {Vec3f{0.2f, 0.4f, 0.6f}, Vec3f{1, 0.5f, 0}, Vec3f{0.01f, 0.02f, 0.03f}, Vec3f{0.9f, 0.9f, 0.1f}})
        for (WhitePoint wp : {WhitePoint::D65, WhitePoint::D50, WhitePoint::A})
        {
            Vec3f back = core::labToSrgb(core::srgbToLab(rgb, wp), wp);
            CHECK(back.x == doctest::Approx(rgb.x).epsilon(1e-4));
            CHECK(back.y == doctest::Approx(rgb.y).epsilon(1e-4));
            CHECK(back.z == doctest::Approx(rgb.z).epsilon(1e-4));
        }
}

TEST_CASE("Color: the white point normalises Lab")
{
    // sRGB white against tungsten white (A) is not neutral: it reads as blue, so b < 0
    Vec3f lab = core::srgbToLab({1, 1, 1}, WhitePoint::A);
    CHECK(lab.x == doctest::Approx(100.0f).epsilon(1e-4));
    CHECK(lab.z < -50.0f);
    CHECK(core::whitePointXYZ(WhitePoint::D65).x == doctest::Approx(0.95047f));
    CHECK(core::whitePointXYZ(WhitePoint::E).z == doctest::Approx(1.0f));
}

TEST_CASE("Color: transfer functions round trip")
{
    for (float v : {0.0f, 0.001f, 0.04045f, 0.2f, 0.5f, 1.0f})
        CHECK(core::linearToSrgb(core::srgbToLinear(v)) == doctest::Approx(v).epsilon(1e-5));
}

TEST_CASE("Color: image converters")
{
    core::Image<std::uint8_t> rgba(2, 1, 4);
    const std::uint8_t px[] = {255, 0, 0, 128, 10, 20, 30, 255};
    std::copy(px, px + 8, rgba.data());

    auto gray = core::toGray8(rgba);
    CHECK(gray.channels() == 1);
    CHECK(gray.colorSpace() == core::ColorSpace::Gray);
    CHECK(gray(0, 0) == 76); // 0.299 * 255
    CHECK(gray(1, 0) == 18); // 0.299*10 + 0.587*20 + 0.114*30 = 18.15

    auto rgb = core::toRGB8(rgba);
    CHECK(rgb.channels() == 3);
    CHECK(rgb(1, 0, 2) == 30);

    auto back = core::toRGBA8(gray);
    CHECK(back.channels() == 4);
    CHECK(back(1, 0, 0) == 18);
    CHECK(back(1, 0, 2) == 18);
    CHECK(back(1, 0, 3) == 255);

    auto again = core::toRGBA8(rgba);
    CHECK(again(0, 0, 3) == 128);

    auto lab = core::toLab(rgba);
    CHECK(lab.colorSpace() == core::ColorSpace::Lab);
    CHECK(lab.channels() == 3);
    CHECK(lab(0, 0, 0) == doctest::Approx(53.2408f).epsilon(1e-3));

    auto rgb8 = core::labToRGB8(lab);
    for (int x = 0; x < 2; ++x)
        for (int c = 0; c < 3; ++c)
            CHECK(rgb8(x, 0, c) == rgba(x, 0, c));

    core::Image<std::uint8_t> five(1, 1, 5);
    CHECK_THROWS(core::toGray8(five));
}
