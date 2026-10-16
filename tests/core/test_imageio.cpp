#include <doctest/doctest.h>

#include "core/ImageIO.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <random>
#include <string>

namespace fs = std::filesystem;

namespace
{
    struct TempDir
    {
        fs::path path;
        TempDir()
        {
            path = fs::temp_directory_path() / ("seething_tests_" + std::to_string(std::random_device{}()));
            fs::create_directories(path);
        }
        ~TempDir() { fs::remove_all(path); }
        std::string file(const char *name) const { return (path / name).string(); }
    };

    core::Image<std::uint8_t> pattern(int w, int h, int c)
    {
        core::Image<std::uint8_t> img(w, h, c);
        for (int y = 0; y < h; ++y)
            for (int x = 0; x < w; ++x)
                for (int k = 0; k < c; ++k)
                    img(x, y, k) = std::uint8_t((x * 37 + y * 11 + k * 60) % 256);
        return img;
    }

    bool samePixels(const core::Image<std::uint8_t> &a, const core::Image<std::uint8_t> &b)
    {
        if (a.width() != b.width() || a.height() != b.height() || a.channels() != b.channels())
            return false;
        return std::equal(a.data(), a.data() + a.size(), b.data());
    }
}

TEST_CASE("ImageIO: PNG round trip is lossless and keeps rows top-down")
{
    TempDir dir;
    for (int c : {1, 3, 4})
    {
        core::Image<std::uint8_t> img = pattern(7, 5, c);
        std::string path = dir.file("round.png");
        std::string error;
        REQUIRE_MESSAGE(core::saveImage(path, img, &error), error);

        core::LoadResult loaded = core::loadImage(path, 0);
        REQUIRE_MESSAGE(loaded.ok(), loaded.error);
        CHECK(loaded.fileChannels == c);
        CHECK(samePixels(loaded.image, img));
    }
}

TEST_CASE("ImageIO: loading forces the asked channel count")
{
    TempDir dir;
    std::string path = dir.file("rgb.png");
    REQUIRE(core::saveImage(path, pattern(4, 4, 3)));

    core::LoadResult rgba = core::loadImage(path);
    REQUIRE(rgba.ok());
    CHECK(rgba.fileChannels == 3);
    CHECK(rgba.image.channels() == 4);
    CHECK(rgba.image(2, 3, 3) == 255);
    CHECK(rgba.image(2, 3, 1) == pattern(4, 4, 3)(2, 3, 1));
}

TEST_CASE("ImageIO: a sub-view saves only its pixels")
{
    TempDir dir;
    core::Image<std::uint8_t> img = pattern(8, 6, 3);
    auto sub = img.view().sub(2, 1, 4, 3);
    for (const char *name : {"sub.png", "sub.bmp", "sub.tga"})
    {
        std::string path = dir.file(name);
        REQUIRE(core::saveImage(path, sub));
        core::LoadResult loaded = core::loadImage(path, 3);
        REQUIRE_MESSAGE(loaded.ok(), loaded.error);
        REQUIRE(loaded.image.width() == 4);
        REQUIRE(loaded.image.height() == 3);
        for (int y = 0; y < 3; ++y)
            for (int x = 0; x < 4; ++x)
                for (int k = 0; k < 3; ++k)
                    CHECK(loaded.image(x, y, k) == sub(x, y, k));
    }
}

TEST_CASE("ImageIO: JPG is close, not exact")
{
    TempDir dir;
    core::Image<std::uint8_t> img(16, 16, 3);
    img.fill(120);
    std::string path = dir.file("flat.JPG");
    REQUIRE(core::saveImage(path, img, nullptr, {90}));
    core::LoadResult loaded = core::loadImage(path, 3);
    REQUIRE(loaded.ok());
    CHECK(std::abs(int(loaded.image(8, 8, 0)) - 120) <= 2);
}

TEST_CASE("ImageIO: failures return an error, never a half-built image")
{
    TempDir dir;
    core::LoadResult missing = core::loadImage(dir.file("missing.png"));
    CHECK_FALSE(missing.ok());
    CHECK(missing.image.empty());
    CHECK(missing.error.find("missing.png") != std::string::npos);

    std::string notAnImage = dir.file("text.png");
    {
        std::FILE *f = std::fopen(notAnImage.c_str(), "wb");
        REQUIRE(f);
        std::fputs("not an image", f);
        std::fclose(f);
    }
    CHECK_FALSE(core::loadImage(notAnImage).ok());
    CHECK_FALSE(core::loadImage(notAnImage, 7).ok());

    std::string error;
    CHECK_FALSE(core::saveImage(dir.file("x.webp"), pattern(2, 2, 3), &error));
    CHECK(error.find("extension") != std::string::npos);
    CHECK_FALSE(core::saveImage(dir.file("empty.png"), core::Image<std::uint8_t>(), &error));
    CHECK_FALSE(core::saveImage((dir.path / "no_dir" / "a.png").string(), pattern(2, 2, 3), &error));
}
