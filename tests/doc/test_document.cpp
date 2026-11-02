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
    Image<std::uint8_t> noise(int width, int height, unsigned seed)
    {
        Image<std::uint8_t> img(width, height, 4);
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

    Document withBackground(int width, int height)
    {
        Document document(width, height);
        document.insertLayer(0, Layer("Background", noise(width, height, 1)));
        document.takeDirty(document.layer(0).id());
        return document;
    }
}

TEST_CASE("Document: insert, move and take keep ids and the active layer")
{
    Document document = withBackground(8, 8);
    const doc::LayerId first = document.layer(0).id();
    const doc::LayerId second = document.insertLayer(1, Layer("B", Image<std::uint8_t>(8, 8, 4)));
    CHECK(first != second);
    CHECK(document.activeLayer()->id() == second);
    document.moveLayer(1, 0);
    CHECK(document.layer(0).id() == second);
    CHECK(document.activeLayer()->id() == second);
    document.takeLayer(0);
    CHECK(document.layerCount() == 1);
    CHECK(document.activeLayer()->id() == first);
    CHECK_THROWS(document.insertLayer(0, Layer("wrong size", Image<std::uint8_t>(4, 8, 4))));
    CHECK_THROWS(document.insertLayer(0, Layer("gray", Image<std::uint8_t>(8, 8, 1))));
}

TEST_CASE("Document: revision moves on every change, dirty rectangles accumulate")
{
    Document document = withBackground(100, 50);
    const doc::LayerId id = document.layer(0).id();
    const auto r0 = document.revision();
    document.pixelsChanged(id, {10, 10, 5, 5});
    document.pixelsChanged(id, {30, 20, 10, 100});
    CHECK(document.revision() > r0);
    CHECK(document.takeDirty(id) == core::Recti{10, 10, 30, 40});
    CHECK(document.takeDirty(id).empty());
}

TEST_CASE("PixelEditRecorder: undo and redo restore every pixel, and only changed tiles are kept")
{
    Document document = withBackground(200, 70); // partial tiles at the right and bottom edges
    const doc::LayerId id = document.layer(0).id();
    const Image<std::uint8_t> before = document.layer(0).pixels();

    doc::PixelEditRecorder rec(document, id);
    const core::Recti rect{60, 5, 90, 65};
    rec.touch(rect);
    for (int y = rect.y; y < rect.bottom(); ++y)
        for (int x = rect.x; x < rect.right(); ++x)
            rec.image()(x, y, 0) = std::uint8_t(x ^ y);
    // A touched tile that ends up unchanged is not stored
    rec.touch({195, 0, 1, 1});
    REQUIRE(rec.commit("Paint"));
    const Image<std::uint8_t> after = document.layer(0).pixels();

    REQUIRE(document.history().size() == 1);
    CHECK(document.history().step(0).label() == "Paint");
    // Columns 0..2, rows 0..1 (the bottom row 6 pixels high); before and after, RGBA
    CHECK(document.history().step(0).bytes() == std::size_t(3 * 64 * 64 + 3 * 64 * 6) * 4 * 2);

    document.undo();
    CHECK(same(document.layer(0).pixels(), before));
    document.redo();
    CHECK(same(document.layer(0).pixels(), after));
}

TEST_CASE("PixelEditRecorder: no change, no step")
{
    Document document = withBackground(16, 16);
    doc::PixelEditRecorder rec(document, document.layer(0).id());
    rec.touch(document.bounds());
    CHECK_FALSE(rec.commit("Nothing"));
    CHECK(document.history().size() == 0);
}

TEST_CASE("Edits: layer steps undo in order")
{
    Document document = withBackground(8, 8);
    const doc::LayerId bg = document.layer(0).id();
    const doc::LayerId added = doc::addEmptyLayer(document);
    const doc::LayerId copy = doc::duplicateLayer(document, 0);
    CHECK(document.layerCount() == 3);
    CHECK(document.layer(1).id() == copy);
    CHECK(same(document.layer(1).pixels(), document.layer(0).pixels()));

    doc::moveLayer(document, 1, 2);
    CHECK(document.layer(2).id() == copy);
    doc::LayerProps props = document.layer(2).props();
    props.opacity = 0.25f;
    props.blend = core::BlendMode::Multiply;
    doc::setLayerProps(document, 2, props, "Opacity");
    doc::removeLayer(document, document.indexOf(added));
    CHECK(document.layerCount() == 2);
    CHECK(document.history().size() == 5);

    document.undo(); // remove
    CHECK(document.indexOf(added) >= 0);
    document.undo(); // props
    CHECK(document.find(copy)->props().opacity == 1.0f);
    document.undo(); // move
    CHECK(document.layer(1).id() == copy);
    document.undo(); // duplicate
    CHECK(document.find(copy) == nullptr);
    document.undo(); // add
    CHECK(document.layerCount() == 1);
    CHECK(document.activeLayer()->id() == bg);

    for (int i = 0; i < 5; ++i)
        document.redo();
    CHECK(document.layerCount() == 2);
    CHECK(document.find(copy)->props().blend == core::BlendMode::Multiply);
    CHECK(document.find(added) == nullptr);
}

TEST_CASE("Edits: a live change is recorded once, from where it started")
{
    Document document = withBackground(4, 4);
    const doc::LayerProps start = document.layer(0).props();
    doc::LayerProps props = start;
    for (float opacity : {0.9f, 0.6f, 0.3f})
    {
        props.opacity = opacity;
        document.setProps(0, props);
    }
    doc::recordLayerProps(document, 0, start, "Opacity");
    CHECK(document.history().size() == 1);
    document.undo();
    CHECK(document.layer(0).props().opacity == 1.0f);
    document.redo();
    CHECK(document.layer(0).props().opacity == 0.3f);
}

TEST_CASE("Edits: selection all, none and invert")
{
    Document document = withBackground(70, 70);
    CHECK_FALSE(document.hasSelection());
    CHECK_FALSE(doc::selectNone(document)); // already empty: no step
    CHECK(doc::selectAll(document));
    CHECK(document.hasSelection());
    CHECK(doc::invertSelection(document));
    CHECK_FALSE(document.hasSelection());
    document.undo();
    CHECK(document.hasSelection());
    CHECK(document.selection()(69, 69) == 255);
    document.undo();
    CHECK_FALSE(document.hasSelection());
    CHECK(document.takeSelectionDirty() == document.bounds());
}

TEST_CASE("Edits: replacePixels converts to RGBA and undoes")
{
    Document document = withBackground(5, 3);
    const Image<std::uint8_t> before = document.layer(0).pixels();
    Image<std::uint8_t> rgb(5, 3, 3);
    rgb.fill(200);
    REQUIRE(doc::replacePixels(document, document.layer(0).id(), rgb, "Copy result"));
    CHECK(document.layer(0).pixels()(4, 2, 0) == 200);
    CHECK(document.layer(0).pixels()(4, 2, 3) == 255);
    document.undo();
    CHECK(same(document.layer(0).pixels(), before));
    CHECK_THROWS(doc::replacePixels(document, document.layer(0).id(), Image<std::uint8_t>(4, 3, 3), "wrong"));
}

namespace
{
    class Sized : public doc::Command
    {
    public:
        explicit Sized(std::size_t bytes) : m_bytes(bytes) {}
        std::string label() const override { return std::to_string(m_bytes); }
        void undo(Document &) override {}
        void redo(Document &) override {}
        std::size_t bytes() const override { return m_bytes; }

    private:
        std::size_t m_bytes;
    };
}

TEST_CASE("History: a new step drops the redo ones, the budget drops the oldest")
{
    Document document;
    doc::History history(100);
    history.record(std::make_unique<Sized>(40));
    history.record(std::make_unique<Sized>(40));
    history.undo(document);
    history.record(std::make_unique<Sized>(30));
    CHECK(history.size() == 2);
    CHECK(history.step(1).label() == "30");
    CHECK_FALSE(history.canRedo());

    history.record(std::make_unique<Sized>(50)); // 120 > 100: the 40 goes
    CHECK(history.size() == 2);
    CHECK(history.dropped() == 1);
    CHECK(history.bytes() == 80);
    history.record(std::make_unique<Sized>(500)); // bigger than the budget alone: kept, everything older goes
    CHECK(history.size() == 1);
    CHECK(history.cursor() == 1);
    CHECK(history.bytes() == 500);
}
