#include <doctest/doctest.h>

#include "core/Geometry.h"
#include "core/Image.h"

#include <cstdint>
#include <stdexcept>
#include <utility>

using core::Image;

namespace
{
    Image<std::uint8_t> ramp(int w, int h, int c)
    {
        Image<std::uint8_t> img(w, h, c);
        for (int y = 0; y < h; ++y)
            for (int x = 0; x < w; ++x)
                for (int k = 0; k < c; ++k)
                    img(x, y, k) = std::uint8_t(y * 100 + x * 10 + k);
        return img;
    }
}

TEST_CASE("Image: construction zero-fills and reports its format")
{
    Image<std::uint8_t> img(3, 2, 4);
    CHECK(img.width() == 3);
    CHECK(img.height() == 2);
    CHECK(img.channels() == 4);
    CHECK(img.size() == 24);
    CHECK(img.format() == core::PixelFormat::RGBA8);
    for (std::size_t i = 0; i < img.size(); ++i)
        CHECK(img.data()[i] == 0);

    CHECK(Image<float>(1, 1, 3).format() == core::PixelFormat::RGB32F);
    CHECK(Image<std::int32_t>(1, 1, 1, core::ColorSpace::Labels).format() == core::PixelFormat::R32I);

    Image<std::uint8_t> none;
    CHECK(none.empty());
    CHECK(none.size() == 0);
    CHECK(Image<std::uint8_t>(0, 5, 3).empty());
    CHECK_THROWS_AS(Image<std::uint8_t>(-1, 2, 3), std::invalid_argument);
}

TEST_CASE("Image: x is the column, y the row, channels interleaved")
{
    Image<std::uint8_t> img = ramp(4, 3, 3);
    CHECK(img(2, 1, 0) == 120);
    CHECK(img(2, 1, 2) == 122);
    // (y * width + x) * channels + c
    CHECK(img.data()[(1 * 4 + 2) * 3 + 2] == 122);
    CHECK(img.row(2)[0] == 200);
}

TEST_CASE("Image: at() checks bounds")
{
    Image<std::uint8_t> img(2, 2, 1);
    CHECK_NOTHROW(img.at(1, 1));
    CHECK_THROWS_AS(img.at(2, 0), std::out_of_range);
    CHECK_THROWS_AS(img.at(0, -1), std::out_of_range);
    CHECK_THROWS_AS(img.at(0, 0, 1), std::out_of_range);
    CHECK(img.inside(1, 1));
    CHECK_FALSE(img.inside(1, 2));
}

TEST_CASE("Image: copies are deep, moves steal the buffer")
{
    Image<std::uint8_t> a = ramp(3, 3, 2);
    Image<std::uint8_t> b = a;
    REQUIRE(b.data() != a.data());
    b(0, 0, 0) = 99;
    CHECK(a(0, 0, 0) == 0);

    const std::uint8_t *buffer = a.data();
    Image<std::uint8_t> c = std::move(a);
    CHECK(c.data() == buffer);
    CHECK(a.empty());
    CHECK(a.width() == 0);

    // Copy-assign over an existing buffer, then self-assign
    Image<std::uint8_t> d(1, 1, 1);
    d = c;
    CHECK(d.width() == 3);
    CHECK(d(2, 2, 1) == 221);
    d = d;
    CHECK(d(2, 2, 1) == 221);

    // Move-assign keeps the colour space
    Image<float> lab(2, 2, 3, core::ColorSpace::Lab);
    Image<float> e;
    e = std::move(lab);
    CHECK(e.colorSpace() == core::ColorSpace::Lab);
    CHECK(e.width() == 2);
}

TEST_CASE("ImageView: sub-views share pixels and keep the parent stride")
{
    Image<std::uint8_t> img = ramp(5, 4, 3);
    core::ImageView<std::uint8_t> v = img.view();
    CHECK(v.stride() == 15);

    core::ImageView<std::uint8_t> sub = v.sub(1, 2, 3, 2);
    CHECK(sub.width() == 3);
    CHECK(sub.height() == 2);
    CHECK(sub.stride() == 15);
    CHECK(sub(0, 0, 0) == img(1, 2, 0));
    CHECK(sub(2, 1, 2) == img(3, 3, 2));

    sub(1, 1, 1) = 7;
    CHECK(img(2, 3, 1) == 7);

    CHECK_THROWS_AS(v.sub(3, 0, 3, 1), std::out_of_range);
    core::ImageView<const std::uint8_t> constView = sub;
    CHECK(constView(2, 1, 2) == img(3, 3, 2));
}

TEST_CASE("Mat3f: product, inverse and singular matrices")
{
    core::Mat3f m = {{{{2, 0, 1}, {1, 3, 0}, {0, 1, 4}}}};
    core::Vec3f v = m * core::Vec3f{1, 2, 3};
    CHECK(v.x == doctest::Approx(5));
    CHECK(v.y == doctest::Approx(7));
    CHECK(v.z == doctest::Approx(14));

    auto inv = m.inverse();
    REQUIRE(inv);
    core::Mat3f id = m * *inv;
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            CHECK(id.m[i][j] == doctest::Approx(i == j ? 1.0f : 0.0f).epsilon(1e-5));

    core::Mat3f singular = {{{{1, 2, 3}, {2, 4, 6}, {0, 1, 1}}}};
    CHECK_FALSE(singular.inverse());
}
