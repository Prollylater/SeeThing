#include <doctest/doctest.h>

#include "doc/Edits.h"
#include "doc/Selection.h"

#include <cstdint>

using core::Image;
using doc::Document;
using doc::SelectMode;

namespace
{
    Document blank(int w, int h)
    {
        Document d(w, h);
        d.insertLayer(0, doc::Layer("Background", Image<std::uint8_t>(w, h, 4)));
        return d;
    }

    int countInside(const Image<std::uint8_t> &mask)
    {
        int n = 0;
        for (std::size_t i = 0; i < mask.size(); ++i)
            n += mask.data()[i] >= 128;
        return n;
    }
}

TEST_CASE("Shapes: rectangle clipped to the image, bounds exact")
{
    const doc::Shape s = doc::rectShape(10, 8, {-2, 3, 5, 10});
    CHECK(s.bounds == core::Recti{0, 3, 3, 5});
    CHECK(countInside(s.mask) == 15);
}

TEST_CASE("Shapes: ellipse is symmetric and inside its box")
{
    const doc::Shape s = doc::ellipseShape(20, 20, {2, 4, 12, 8});
    CHECK(s.bounds.x >= 2);
    CHECK(s.bounds.right() <= 14);
    CHECK(s.mask(8, 8) == 255);  // centre
    CHECK(s.mask(2, 4) == 0);    // box corner
    for (int y = 4; y < 12; ++y)
        for (int x = 2; x < 14; ++x)
            CHECK(s.mask(x, y) == s.mask(2 + 13 - x, y)); // mirror across x = 8
}

TEST_CASE("Shapes: a polygon square covers the pixels whose centres are inside")
{
    const core::Vec2f square[] = {{2, 2}, {6, 2}, {6, 5}, {2, 5}};
    const doc::Shape s = doc::polygonShape(10, 10, square);
    CHECK(s.bounds == core::Recti{2, 2, 4, 3});
    CHECK(countInside(s.mask) == 12);

    // A bow tie crosses itself; the even-odd rule keeps both triangles
    const core::Vec2f bow[] = {{0, 0}, {8, 8}, {8, 0}, {0, 8}};
    const doc::Shape b = doc::polygonShape(8, 8, bow);
    CHECK(b.mask(1, 4) == 255);
    CHECK(b.mask(4, 1) == 0);
}

TEST_CASE("applySelection: replace, add, subtract, intersect, each one step")
{
    Document d = blank(16, 16);
    CHECK(doc::applySelection(d, doc::rectShape(16, 16, {0, 0, 8, 16}), SelectMode::Replace, "Rect"));
    CHECK(countInside(d.selection()) == 128);
    CHECK(doc::applySelection(d, doc::rectShape(16, 16, {8, 0, 4, 16}), SelectMode::Add, "Add"));
    CHECK(countInside(d.selection()) == 192);
    CHECK(doc::applySelection(d, doc::rectShape(16, 16, {0, 0, 16, 8}), SelectMode::Subtract, "Sub"));
    CHECK(countInside(d.selection()) == 96);
    CHECK(doc::applySelection(d, doc::rectShape(16, 16, {0, 8, 4, 8}), SelectMode::Intersect, "Int"));
    CHECK(countInside(d.selection()) == 32);
    CHECK_FALSE(doc::applySelection(d, doc::rectShape(16, 16, {0, 8, 4, 8}), SelectMode::Add, "Again"));
    CHECK(d.history().size() == 4);
    d.undo();
    CHECK(countInside(d.selection()) == 96);
}

TEST_CASE("fillMask: paints inside the mask and the selection only")
{
    Document d = blank(8, 8);
    const doc::LayerId id = d.layer(0).id();
    doc::applySelection(d, doc::rectShape(8, 8, {0, 0, 4, 8}), SelectMode::Replace, "Left");
    const doc::Shape region = doc::rectShape(8, 8, {2, 2, 4, 4});
    CHECK(doc::fillMask(d, id, region.mask, region.bounds, {10, 20, 30, 255}, "Fill"));
    const Image<std::uint8_t> &px = d.layer(0).pixels();
    CHECK(px(3, 3, 0) == 10);
    CHECK(px(3, 3, 3) == 255);
    CHECK(px(4, 3, 0) == 0); // outside the selection
    CHECK(px(1, 3, 0) == 0); // outside the mask
    d.undo();
    CHECK(d.layer(0).pixels()(3, 3, 0) == 0);
}

TEST_CASE("Seeds: add, remove the nearest, clear, all undoable")
{
    Document d = blank(20, 20);
    doc::addSeed(d, {3, 3});
    doc::addSeed(d, {10, 10});
    doc::addSeed(d, {10, 10}); // already there: no step
    doc::addSeed(d, {25, 0});  // outside: no step
    CHECK(d.seeds().size() == 2);
    CHECK(d.history().size() == 2);
    CHECK_FALSE(doc::removeSeedNear(d, {16, 16}, 3));
    CHECK(doc::removeSeedNear(d, {9.6f, 11}, 3));
    CHECK(d.seeds() == std::vector<core::Vec2i>{{3, 3}});
    doc::clearSeeds(d);
    CHECK(d.seeds().empty());
    d.undo();
    d.undo();
    CHECK(d.seeds().size() == 2);
}
