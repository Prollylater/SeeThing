#include <doctest/doctest.h>

#include "gpu/DebugOutput.h"
#include "gpu/Framebuffer.h"
#include "gpu/Renderer.h"
#include "gpu/ShaderCache.h"
#include "gpu/Texture.h"
#include "core/Stroke.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
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

TEST_CASE("Texture: rectangles upload and read back in place")
{
    gpu::Texture tex = gpu::Texture::fromImage<std::uint8_t>(pattern(9, 7, 4));
    Image<std::uint8_t> patch = pattern(3, 2, 4);
    patch.fill(7);
    tex.upload(4, 5, patch);
    CHECK(samePixels(tex.download8({4, 5, 3, 2}), patch));
    const Image<std::uint8_t> all = tex.download8();
    CHECK(all(3, 5, 0) == pattern(9, 7, 4)(3, 5, 0));
    CHECK(all(6, 6, 2) == 7);
    CHECK_THROWS_AS(tex.upload(7, 5, patch), std::invalid_argument);
    CHECK_THROWS_AS(tex.download8({0, 0, 10, 1}), std::invalid_argument);
}

namespace
{
    // The same W3C formula as composite.frag, in float
    std::array<float, 4> referenceOver(std::array<float, 4> b, std::array<float, 4> s, float opacity, core::BlendMode mode)
    {
        auto blend = [mode](float cb, float cs) {
            switch (mode)
            {
            case core::BlendMode::Multiply: return cb * cs;
            case core::BlendMode::Screen: return cb + cs - cb * cs;
            case core::BlendMode::Overlay: return cb < 0.5f ? 2 * cb * cs : 1 - 2 * (1 - cb) * (1 - cs);
            case core::BlendMode::Difference: return std::fabs(cb - cs);
            case core::BlendMode::Normal: break;
            }
            return cs;
        };
        const float as = s[3] * opacity, ab = b[3];
        const float ao = as + ab * (1 - as);
        std::array<float, 4> out{0, 0, 0, ao};
        for (int k = 0; k < 3; ++k)
        {
            const float mixed = (1 - ab) * s[k] + ab * blend(b[k], s[k]);
            out[k] = ao > 0 ? (as * mixed + ab * (1 - as) * b[k]) / ao : 0;
        }
        return out;
    }
}

TEST_CASE("Renderer: composite matches the W3C formulas in every mode")
{
    gpu::Renderer renderer(SEETHING_SHADER_DIR);
    const int w = 16, h = 16;
    Image<std::uint8_t> back(w, h, 4), src(w, h, 4);
    std::mt19937 rng(3);
    for (std::size_t i = 0; i < back.size(); ++i)
    {
        back.data()[i] = std::uint8_t(rng());
        src.data()[i] = std::uint8_t(rng());
    }
    // Opaque and fully transparent pixels too
    for (int x = 0; x < w; ++x)
    {
        back(x, 0, 3) = 255;
        src(x, 1, 3) = 0;
        back(x, 2, 3) = 0;
    }
    gpu::Texture backTex = gpu::Texture::fromImage<std::uint8_t>(back), srcTex = gpu::Texture::fromImage<std::uint8_t>(src);
    gpu::Texture dst = blankRGBA(w, h);
    gpu::Framebuffer fbo(dst);

    for (core::BlendMode mode : core::blendModes)
        for (float opacity : {1.0f, 0.4f})
        {
            CAPTURE(core::blendModeName(mode));
            CAPTURE(opacity);
            renderer.composite(backTex, srcTex, opacity, mode, fbo);
            const Image<std::uint8_t> out = dst.download8();
            int worst = 0;
            for (int y = 0; y < h; ++y)
                for (int x = 0; x < w; ++x)
                {
                    std::array<float, 4> b, s;
                    for (int k = 0; k < 4; ++k)
                    {
                        b[k] = back(x, y, k) / 255.0f;
                        s[k] = src(x, y, k) / 255.0f;
                    }
                    const std::array<float, 4> want = referenceOver(b, s, opacity, mode);
                    // Colour is meaningless where nothing is left
                    const int channels = out(x, y, 3) == 0 ? 3 : 0;
                    for (int k = channels; k < 4; ++k)
                        worst = std::max(worst, std::abs(int(out(x, y, k)) - int(std::lround(want[k] * 255.0f))));
                }
            CHECK(worst <= 1);
        }
}

TEST_CASE("Renderer: clear fills the target")
{
    gpu::Renderer renderer(SEETHING_SHADER_DIR);
    gpu::Texture dst = gpu::Texture::fromImage<std::uint8_t>(pattern(3, 3, 4));
    gpu::Framebuffer fbo(dst);
    renderer.clear(fbo);
    const Image<std::uint8_t> out = dst.download8();
    CHECK(std::all_of(out.data(), out.data() + out.size(), [](std::uint8_t v) { return v == 0; }));
}

TEST_CASE("Renderer: points respect a clip mask")
{
    gpu::Renderer renderer(SEETHING_SHADER_DIR);
    gpu::Texture dst = blankRGBA(8, 4);
    gpu::Framebuffer fbo(dst);
    // Left half selected
    Image<std::uint8_t> mask(8, 4, 1);
    for (int y = 0; y < 4; ++y)
        for (int x = 0; x < 4; ++x)
            mask(x, y) = 255;
    gpu::Texture clip = gpu::Texture::fromImage<std::uint8_t>(mask);

    const core::Vec2f row[] = {{1, 1}, {2, 1}, {3, 1}, {4, 1}, {5, 1}, {6, 1}};
    renderer.drawPoints(fbo, row, {1, 1, 1}, 1.0f, &clip);
    const Image<std::uint8_t> out = dst.download8();
    for (int x = 1; x <= 6; ++x)
    {
        CAPTURE(x);
        CHECK(out(x, 1, 0) == (x < 4 ? 255 : 0));
    }
}

TEST_CASE("Renderer: a hard dab covers its disc, a soft one fades out")
{
    gpu::Renderer renderer(SEETHING_SHADER_DIR);
    gpu::Texture cov(40, 40, PixelFormat::R32F);
    gpu::Framebuffer fbo(cov);
    renderer.clear(fbo);
    const core::Vec2f centre[] = {{20, 20}};
    renderer.drawDabs(fbo, centre, 20.0f, 1.0f, 1.0f);
    Image<float> out = cov.downloadFloat();
    double area = 0;
    for (std::size_t i = 0; i < out.size(); ++i)
        area += out.data()[i];
    // A radius 10 disc, antialiased rim included
    CHECK(area == doctest::Approx(3.14159 * 100).epsilon(0.02));
    CHECK(out(20, 20) == doctest::Approx(1.0f));
    CHECK(out(20, 31) == 0.0f);
    CHECK(out(15, 20) == doctest::Approx(out(24, 20))); // symmetric about x = 20

    renderer.clear(fbo);
    renderer.drawDabs(fbo, centre, 20.0f, 0.0f, 1.0f);
    out = cov.downloadFloat();
    CHECK(out(20, 20) > 0.95f);
    CHECK(out(20, 24) < out(20, 22));
    CHECK(out(20, 28) < 0.15f);
}

TEST_CASE("Renderer: flow builds up over repeated dabs and never passes 1")
{
    gpu::Renderer renderer(SEETHING_SHADER_DIR);
    gpu::Texture cov(8, 8, PixelFormat::R32F);
    gpu::Framebuffer fbo(cov);
    renderer.clear(fbo);
    const core::Vec2f centre[] = {{4, 4}};
    renderer.drawDabs(fbo, centre, 6.0f, 1.0f, 0.5f);
    CHECK(cov.downloadFloat()(3, 3) == doctest::Approx(0.5f));
    renderer.drawDabs(fbo, centre, 6.0f, 1.0f, 0.5f);
    CHECK(cov.downloadFloat()(3, 3) == doctest::Approx(0.75f)); // 0.5 + 0.5 * (1 - 0.5)
    for (int i = 0; i < 30; ++i)
        renderer.drawDabs(fbo, centre, 6.0f, 1.0f, 0.5f);
    const float many = cov.downloadFloat()(3, 3);
    CHECK(many <= 1.0f);
    CHECK(many > 0.999f);
    // Low flow still adds up in float
    renderer.clear(fbo);
    for (int i = 0; i < 100; ++i)
        renderer.drawDabs(fbo, centre, 6.0f, 1.0f, 0.01f);
    CHECK(cov.downloadFloat()(3, 3) == doctest::Approx(1.0 - std::pow(0.99, 100)).epsilon(0.01));
}

TEST_CASE("Renderer: applyStroke paints over and erases with destination-out")
{
    gpu::Renderer renderer(SEETHING_SHADER_DIR);
    Image<std::uint8_t> layerPx(2, 1, 4);
    for (int k = 0; k < 4; ++k)
    {
        layerPx(0, 0, k) = k == 3 ? 255 : 0;   // opaque black
        layerPx(1, 0, k) = k == 3 ? 0 : 0;     // transparent
    }
    gpu::Texture layer = gpu::Texture::fromImage<std::uint8_t>(layerPx);
    Image<float> full(2, 1, 1);
    full.fill(1.0f);
    gpu::Texture cov = gpu::Texture::fromImage<float>(full);
    gpu::Texture dst = blankRGBA(2, 1);
    gpu::Framebuffer fbo(dst);

    renderer.applyStroke(layer, cov, nullptr, {1, 0, 0}, 0.5f, false, fbo);
    Image<std::uint8_t> out = dst.download8();
    CHECK(out(0, 0, 0) == doctest::Approx(128).epsilon(0.02)); // half red over black
    CHECK(out(0, 0, 3) == 255);
    CHECK(out(1, 0, 0) == 255);                                // red over nothing keeps its colour
    CHECK(out(1, 0, 3) == doctest::Approx(128).epsilon(0.02)); // at half alpha

    renderer.applyStroke(layer, cov, nullptr, {1, 0, 0}, 0.5f, true, fbo);
    out = dst.download8();
    CHECK(out(0, 0, 0) == 0);
    CHECK(out(0, 0, 3) == doctest::Approx(128).epsilon(0.02));
}

TEST_CASE("Brush: a fast stroke has no gaps")
{
    gpu::Renderer renderer(SEETHING_SHADER_DIR);
    gpu::Texture cov(120, 40, PixelFormat::R32F);
    gpu::Framebuffer fbo(cov);
    renderer.clear(fbo);
    // Two pointer samples 100 pixels apart, as a fast drag gives in one frame
    core::DabSpacer spacer;
    std::vector<core::Vec2f> dabs;
    spacer.begin({10, 20}, dabs);
    spacer.moveTo({110, 20}, 6.0f * 0.25f, dabs);
    renderer.drawDabs(fbo, dabs, 6.0f, 0.8f, 1.0f);
    const Image<float> out = cov.downloadFloat();
    float weakest = 1.0f;
    for (int x = 10; x < 110; ++x)
        weakest = std::min(weakest, out(x, 19));
    CHECK(weakest > 0.99f);
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
