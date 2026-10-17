#include <doctest/doctest.h>

#include "gpu/DebugOutput.h"
#include "gpu/Framebuffer.h"
#include "gpu/Renderer.h"
#include "gpu/ShaderCache.h"
#include "gpu/Texture.h"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <random>
#include <stdexcept>
#include <vector>

namespace fs = std::filesystem;
using core::Image;
using core::PixelFormat;

namespace
{
    Image<std::uint8_t> pattern(int w, int h, int c)
    {
        Image<std::uint8_t> img(w, h, c);
        for (int y = 0; y < h; ++y)
            for (int x = 0; x < w; ++x)
                for (int k = 0; k < c; ++k)
                    img(x, y, k) = std::uint8_t((x * 37 + y * 11 + k * 60 + 1) % 256);
        return img;
    }

    bool samePixels(core::ImageView<const std::uint8_t> a, core::ImageView<const std::uint8_t> b)
    {
        if (a.width() != b.width() || a.height() != b.height() || a.channels() != b.channels())
            return false;
        for (int y = 0; y < a.height(); ++y)
            if (!std::equal(a.row(y), a.row(y) + std::size_t(a.width()) * a.channels(), b.row(y)))
                return false;
        return true;
    }

    gpu::Texture blankRGBA(int w, int h)
    {
        return gpu::Texture::fromImage<std::uint8_t>(Image<std::uint8_t>(w, h, 4));
    }

    const char *vertexCode = "#version 450 core\nvoid main() { gl_Position = vec4(0.0, 0.0, 0.0, 1.0); }\n";
    const char *fragmentCode = "#version 450 core\nout vec4 c;\nvoid main() { c = vec4(1.0); }\n";

    void writeFile(const fs::path &path, const std::string &text)
    {
        std::ofstream(path, std::ios::binary) << text;
    }
}

TEST_CASE("GlHandle: owns one name, moves it, deletes it")
{
    GLuint id = 0;
    {
        gpu::BufferHandle a = gpu::BufferHandle::create();
        id = a.get();
        REQUIRE(id != 0);
        CHECK(glIsBuffer(id));
        gpu::BufferHandle b = std::move(a);
        CHECK(a.get() == 0);
        CHECK(b.get() == id);
        gpu::BufferHandle c;
        c = std::move(b);
        CHECK(c.get() == id);
    }
    CHECK_FALSE(glIsBuffer(id));
}

TEST_CASE("Texture: round trip keeps every pixel at any width and channel count")
{
    // Odd widths break the default 4-byte row alignment
    for (int c : {1, 2, 3, 4})
        for (int w : {1, 5, 7, 13})
        {
            CAPTURE(c);
            CAPTURE(w);
            Image<std::uint8_t> img = pattern(w, 3, c);
            gpu::Texture tex = gpu::Texture::fromImage<std::uint8_t>(img);
            CHECK(tex.width() == w);
            CHECK(tex.height() == 3);
            CHECK(tex.channels() == c);
            CHECK(tex.format() == core::pixelFormatOf<std::uint8_t>(c));
            CHECK(samePixels(tex.download8(), img));
        }
}

TEST_CASE("Texture: a sub-view uploads only its pixels")
{
    Image<std::uint8_t> img = pattern(9, 7, 3);
    auto sub = img.view().sub(2, 1, 5, 4);
    gpu::Texture tex = gpu::Texture::fromImage<std::uint8_t>(sub);
    CHECK(samePixels(tex.download8(), sub));
}

TEST_CASE("Texture: mismatched uploads and bad sizes throw")
{
    gpu::Texture tex(4, 4, PixelFormat::RGBA8);
    CHECK_THROWS_AS(tex.upload(pattern(4, 4, 3)), std::invalid_argument);
    CHECK_THROWS_AS(tex.upload(pattern(3, 4, 4)), std::invalid_argument);
    CHECK_THROWS_AS(gpu::Texture(0, 4, PixelFormat::R8), std::invalid_argument);
    CHECK_THROWS_AS(gpu::Texture(2, 2, PixelFormat::Unknown), std::invalid_argument);
    CHECK(gpu::Texture().empty());
    CHECK(gpu::Texture().download8().empty());

    gpu::Texture f(3, 2, PixelFormat::R32F);
    Image<float> values(3, 2, 1);
    values.fill(0.25f);
    CHECK_NOTHROW(f.upload(values));
    CHECK_THROWS_AS(f.download8(), std::logic_error);
}

TEST_CASE("Renderer: blit copies row 0 to row 0 and shows gray as gray")
{
    gpu::Renderer renderer(SEETHING_SHADER_DIR);

    Image<std::uint8_t> rgb = pattern(5, 3, 3);
    gpu::Texture src = gpu::Texture::fromImage<std::uint8_t>(rgb);
    gpu::Texture dst = blankRGBA(5, 3);
    gpu::Framebuffer fbo(dst);
    renderer.blit(src, fbo);
    Image<std::uint8_t> out = dst.download8();
    for (int y = 0; y < 3; ++y)
        for (int x = 0; x < 5; ++x)
        {
            CHECK(out(x, y, 0) == rgb(x, y, 0));
            CHECK(out(x, y, 2) == rgb(x, y, 2));
            CHECK(out(x, y, 3) == 255);
        }

    Image<std::uint8_t> gray = pattern(5, 3, 1);
    renderer.blit(gpu::Texture::fromImage<std::uint8_t>(gray), fbo);
    out = dst.download8();
    CHECK(out(3, 2, 0) == gray(3, 2));
    CHECK(out(3, 2, 1) == gray(3, 2));
    CHECK(out(3, 2, 2) == gray(3, 2));
}

TEST_CASE("Renderer: points land on image pixels, top-left origin")
{
    gpu::Renderer renderer(SEETHING_SHADER_DIR);
    gpu::Texture dst = blankRGBA(8, 6);
    gpu::Framebuffer fbo(dst);

    const core::Vec2f one[] = {{2, 1}};
    renderer.drawPoints(fbo, one, {1, 0, 0}, 1.0f);
    Image<std::uint8_t> out = dst.download8();
    int lit = 0;
    for (int y = 0; y < 6; ++y)
        for (int x = 0; x < 8; ++x)
            lit += out(x, y, 0) != 0;
    CHECK(lit == 1);
    CHECK(out(2, 1, 0) == 255);
    CHECK(out(2, 1, 3) == 255);

    // A size 3 point covers the 3x3 block around its pixel
    const core::Vec2f big[] = {{5, 4}};
    renderer.drawPoints(fbo, big, {0, 1, 0}, 3.0f);
    out = dst.download8();
    for (int y = 3; y <= 5; ++y)
        for (int x = 4; x <= 6; ++x)
            CHECK(out(x, y, 1) == 255);
    CHECK(out(3, 4, 1) == 0);
    CHECK(out(7, 4, 1) == 0);
}

TEST_CASE("Renderer: retired objects live until endFrame")
{
    gpu::Renderer renderer(SEETHING_SHADER_DIR);
    gpu::Texture tex = blankRGBA(2, 2);
    gpu::Framebuffer fbo(tex);
    const GLuint texId = tex.id(), fboId = fbo.id();

    renderer.retire(std::move(fbo));
    renderer.retire(std::move(tex));
    renderer.retire(gpu::Texture());
    CHECK(tex.empty());
    CHECK(renderer.retiredCount() == 2);
    CHECK(glIsTexture(texId));
    CHECK(glIsFramebuffer(fboId));

    renderer.endFrame();
    CHECK(renderer.retiredCount() == 0);
    CHECK_FALSE(glIsTexture(texId));
    CHECK_FALSE(glIsFramebuffer(fboId));
}

TEST_CASE("Program: a compile error gives an empty program and the info log")
{
    std::string log;
    gpu::Program bad = gpu::Program::link({{GL_VERTEX_SHADER, vertexCode, "ok.vert"},
                                           {GL_FRAGMENT_SHADER, "#version 450 core\nvoid main() { oops }\n", "broken.frag"}},
                                          &log);
    CHECK(bad.empty());
    CHECK(log.find("broken.frag") != std::string::npos);
    CHECK(log.size() > std::string("broken.frag (fragment): ").size());

    gpu::Program good = gpu::Program::link({{GL_VERTEX_SHADER, vertexCode, "a"}, {GL_FRAGMENT_SHADER, fragmentCode, "b"}}, &log);
    CHECK_FALSE(good.empty());
    CHECK(log.empty());
}

TEST_CASE("ShaderCache: builds once, reloads changed files, keeps the old program on a bad edit")
{
    const fs::path dir = fs::temp_directory_path() / ("seething_gpu_" + std::to_string(std::random_device{}()));
    fs::create_directories(dir);
    const fs::path vert = dir / "a.vert", frag = dir / "a.frag";
    writeFile(vert, vertexCode);
    writeFile(frag, fragmentCode);

    gpu::ShaderCache cache;
    REQUIRE(cache.load("a", vert.string(), frag.string()));
    const GLuint first = cache.get("a").id();
    CHECK(first != 0);
    CHECK(cache.reloadChanged() == 0);
    CHECK_THROWS_AS(cache.get("missing"), std::out_of_range);

    const auto later = fs::last_write_time(frag) + std::chrono::seconds(2);
    writeFile(frag, "#version 450 core\nout vec4 c;\nvoid main() { c = vec4(0.5); }\n");
    fs::last_write_time(frag, later);
    CHECK(cache.reloadChanged() == 1);
    const GLuint second = cache.get("a").id();
    CHECK(second != 0);
    CHECK_FALSE(glIsProgram(first));

    writeFile(frag, "#version 450 core\nvoid main() { nope }\n");
    fs::last_write_time(frag, later + std::chrono::seconds(2));
    CHECK(cache.reloadChanged() == 0);
    CHECK(cache.get("a").id() == second);
    CHECK_FALSE(cache.lastError().empty());
    // Reported once, not on every check
    CHECK(cache.reloadChanged() == 0);

    CHECK_FALSE(cache.load("b", (dir / "none.vert").string(), frag.string()));
    CHECK_FALSE(cache.contains("b"));
    fs::remove_all(dir);
}

TEST_CASE("DebugOutput: a GL error reaches the callback")
{
    // Nothing before this test may have produced a message
    REQUIRE(gpu::debugMessageCount() == 0);
    glEnable(0xDEAD);
    CHECK(gpu::debugMessageCount() >= 1);
    gpu::resetDebugMessageCount();
}
