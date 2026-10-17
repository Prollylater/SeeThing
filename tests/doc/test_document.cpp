#include <doctest/doctest.h>

#include "doc/Document.h"
#include "doc/Edits.h"

#include <cstdint>
#include <random>

using core::Image;
using doc::Document;
using doc::Layer;

namespace
{
    Image<std::uint8_t> noise(int w, int h, unsigned seed)
    {
        Image<std::uint8_t> img(w, h, 4);
        std::mt19937 rng(seed);
        for (std::size_t i = 0; i < img.size(); ++i)
            img.data()[i] = std::uint8_t(rng());
        return img;
    }

    bool same(const Image<std::uint8_t> &a, const Image<std::uint8_t> &b)
    {
        return a.width() == b.width() && a.height() == b.height() && a.channels() == b.channels() &&
               std::equal(a.data(), a.data() + a.size(), b.data());
    }

    Document withBackground(int w, int h)
    {
        Document d(w, h);
        d.insertLayer(0, Layer("Background", noise(w, h, 1)));
        d.takeDirty(d.layer(0).id());
        return d;
    }
}

TEST_CASE("Document: insert, move and take keep ids and the active layer")
{
    Document d = withBackground(8, 8);
    const doc::LayerId a = d.layer(0).id();
    const doc::LayerId b = d.insertLayer(1, Layer("B", Image<std::uint8_t>(8, 8, 4)));
    CHECK(a != b);
    CHECK(d.activeLayer()->id() == b);
    d.moveLayer(1, 0);
    CHECK(d.layer(0).id() == b);
    CHECK(d.activeLayer()->id() == b);
    d.takeLayer(0);
    CHECK(d.layerCount() == 1);
    CHECK(d.activeLayer()->id() == a);
    CHECK_THROWS(d.insertLayer(0, Layer("wrong size", Image<std::uint8_t>(4, 8, 4))));
    CHECK_THROWS(d.insertLayer(0, Layer("gray", Image<std::uint8_t>(8, 8, 1))));
}

TEST_CASE("Document: revision moves on every change, dirty rectangles accumulate")
{
    Document d = withBackground(100, 50);
    const doc::LayerId id = d.layer(0).id();
    const auto r0 = d.revision();
    d.pixelsChanged(id, {10, 10, 5, 5});
    d.pixelsChanged(id, {30, 20, 10, 100});
    CHECK(d.revision() > r0);
    CHECK(d.takeDirty(id) == core::Recti{10, 10, 30, 40});
    CHECK(d.takeDirty(id).empty());
}

TEST_CASE("PixelEditRecorder: undo and redo restore every pixel, and only changed tiles are kept")
{
    Document d = withBackground(200, 70); // partial tiles at the right and bottom edges
    const doc::LayerId id = d.layer(0).id();
    const Image<std::uint8_t> before = d.layer(0).pixels();

    doc::PixelEditRecorder rec(d, id);
    const core::Recti r{60, 5, 90, 65};
    rec.touch(r);
    for (int y = r.y; y < r.bottom(); ++y)
        for (int x = r.x; x < r.right(); ++x)
            rec.image()(x, y, 0) = std::uint8_t(x ^ y);
    // A touched tile that ends up unchanged is not stored
    rec.touch({195, 0, 1, 1});
    REQUIRE(rec.commit("Paint"));
    const Image<std::uint8_t> after = d.layer(0).pixels();

    REQUIRE(d.history().size() == 1);
    CHECK(d.history().step(0).label() == "Paint");
    // Columns 0..2, rows 0..1 (the bottom row 6 pixels high); before and after, RGBA
    CHECK(d.history().step(0).bytes() == std::size_t(3 * 64 * 64 + 3 * 64 * 6) * 4 * 2);

    d.undo();
    CHECK(same(d.layer(0).pixels(), before));
    d.redo();
    CHECK(same(d.layer(0).pixels(), after));
}

TEST_CASE("PixelEditRecorder: no change, no step")
{
    Document d = withBackground(16, 16);
    doc::PixelEditRecorder rec(d, d.layer(0).id());
    rec.touch(d.bounds());
    CHECK_FALSE(rec.commit("Nothing"));
    CHECK(d.history().size() == 0);
}

TEST_CASE("Edits: layer steps undo in order")
{
    Document d = withBackground(8, 8);
    const doc::LayerId bg = d.layer(0).id();
    const doc::LayerId added = doc::addEmptyLayer(d);
    const doc::LayerId copy = doc::duplicateLayer(d, 0);
    CHECK(d.layerCount() == 3);
    CHECK(d.layer(1).id() == copy);
    CHECK(same(d.layer(1).pixels(), d.layer(0).pixels()));

    doc::moveLayer(d, 1, 2);
    CHECK(d.layer(2).id() == copy);
    doc::LayerProps p = d.layer(2).props();
    p.opacity = 0.25f;
    p.blend = core::BlendMode::Multiply;
    doc::setLayerProps(d, 2, p, "Opacity");
    doc::removeLayer(d, d.indexOf(added));
    CHECK(d.layerCount() == 2);
    CHECK(d.history().size() == 5);

    d.undo(); // remove
    CHECK(d.indexOf(added) >= 0);
    d.undo(); // props
    CHECK(d.find(copy)->props().opacity == 1.0f);
    d.undo(); // move
    CHECK(d.layer(1).id() == copy);
    d.undo(); // duplicate
    CHECK(d.find(copy) == nullptr);
    d.undo(); // add
    CHECK(d.layerCount() == 1);
    CHECK(d.activeLayer()->id() == bg);

    for (int i = 0; i < 5; ++i)
        d.redo();
    CHECK(d.layerCount() == 2);
    CHECK(d.find(copy)->props().blend == core::BlendMode::Multiply);
    CHECK(d.find(added) == nullptr);
}

TEST_CASE("Edits: a live change is recorded once, from where it started")
{
    Document d = withBackground(4, 4);
    const doc::LayerProps start = d.layer(0).props();
    doc::LayerProps p = start;
    for (float o : {0.9f, 0.6f, 0.3f})
    {
        p.opacity = o;
        d.setProps(0, p);
    }
    doc::recordLayerProps(d, 0, start, "Opacity");
    CHECK(d.history().size() == 1);
    d.undo();
    CHECK(d.layer(0).props().opacity == 1.0f);
    d.redo();
    CHECK(d.layer(0).props().opacity == 0.3f);
}

TEST_CASE("Edits: selection all, none and invert")
{
    Document d = withBackground(70, 70);
    CHECK_FALSE(d.hasSelection());
    CHECK_FALSE(doc::selectNone(d)); // already empty: no step
    CHECK(doc::selectAll(d));
    CHECK(d.hasSelection());
    CHECK(doc::invertSelection(d));
    CHECK_FALSE(d.hasSelection());
    d.undo();
    CHECK(d.hasSelection());
    CHECK(d.selection()(69, 69) == 255);
    d.undo();
    CHECK_FALSE(d.hasSelection());
    CHECK(d.takeSelectionDirty() == d.bounds());
}

TEST_CASE("Edits: replacePixels converts to RGBA and undoes")
{
    Document d = withBackground(5, 3);
    const Image<std::uint8_t> before = d.layer(0).pixels();
    Image<std::uint8_t> rgb(5, 3, 3);
    rgb.fill(200);
    REQUIRE(doc::replacePixels(d, d.layer(0).id(), rgb, "Copy result"));
    CHECK(d.layer(0).pixels()(4, 2, 0) == 200);
    CHECK(d.layer(0).pixels()(4, 2, 3) == 255);
    d.undo();
    CHECK(same(d.layer(0).pixels(), before));
    CHECK_THROWS(doc::replacePixels(d, d.layer(0).id(), Image<std::uint8_t>(4, 3, 3), "wrong"));
}

namespace
{
    class Sized : public doc::Command
    {
    public:
        explicit Sized(std::size_t n) : m_n(n) {}
        std::string label() const override { return std::to_string(m_n); }
        void undo(Document &) override {}
        void redo(Document &) override {}
        std::size_t bytes() const override { return m_n; }

    private:
        std::size_t m_n;
    };
}

TEST_CASE("History: a new step drops the redo ones, the budget drops the oldest")
{
    Document d;
    doc::History h(100);
    h.record(std::make_unique<Sized>(40));
    h.record(std::make_unique<Sized>(40));
    h.undo(d);
    h.record(std::make_unique<Sized>(30));
    CHECK(h.size() == 2);
    CHECK(h.step(1).label() == "30");
    CHECK_FALSE(h.canRedo());

    h.record(std::make_unique<Sized>(50)); // 120 > 100: the 40 goes
    CHECK(h.size() == 2);
    CHECK(h.dropped() == 1);
    CHECK(h.bytes() == 80);
    h.record(std::make_unique<Sized>(500)); // bigger than the budget alone: kept, everything older goes
    CHECK(h.size() == 1);
    CHECK(h.cursor() == 1);
    CHECK(h.bytes() == 500);
}
