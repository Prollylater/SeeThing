#include <doctest/doctest.h>

#include "doc/Edits.h"
#include "doc/Selection.h"

#include <cstdint>

using core::Image;
using doc::Document;
using doc::SelectMode;

namespace
{
    Document blank(int width, int height)
    {
        Document document(width, height);
        document.insertLayer(0, doc::Layer("Background", Image<std::uint8_t>(width, height, 4)));
        return document;
    }

    int countInside(const Image<std::uint8_t> &mask)
    {
        int count = 0;
        for (std::size_t i = 0; i < mask.size(); ++i)
            count += mask.data()[i] >= 128;
        return count;
    }
}

TEST_CASE("Shapes: rectangle clipped to the image, bounds exact")
{
    const doc::Shape shape = doc::rectShape(10, 8, {-2, 3, 5, 10});
    CHECK(shape.bounds == core::Recti{0, 3, 3, 5});
    CHECK(countInside(shape.mask) == 15);
}

TEST_CASE("Shapes: ellipse is symmetric and inside its box")
{
    const doc::Shape shape = doc::ellipseShape(20, 20, {2, 4, 12, 8});
    CHECK(shape.bounds.x >= 2);
    CHECK(shape.bounds.right() <= 14);
    CHECK(shape.mask(8, 8) == 255);  // centre
    CHECK(shape.mask(2, 4) == 0);    // box corner
    for (int y = 4; y < 12; ++y)
        for (int x = 2; x < 14; ++x)
            CHECK(shape.mask(x, y) == shape.mask(2 + 13 - x, y)); // mirror across x = 8
}

TEST_CASE("Shapes: a polygon square covers the pixels whose centres are inside")
{
    const core::Vec2f square[] = {{2, 2}, {6, 2}, {6, 5}, {2, 5}};
    const doc::Shape shape = doc::polygonShape(10, 10, square);
    CHECK(shape.bounds == core::Recti{2, 2, 4, 3});
    CHECK(countInside(shape.mask) == 12);

    // A bow tie crosses itself; the even-odd rule keeps both triangles
    const core::Vec2f bow[] = {{0, 0}, {8, 8}, {8, 0}, {0, 8}};
    const doc::Shape bowTie = doc::polygonShape(8, 8, bow);
    CHECK(bowTie.mask(1, 4) == 255);
    CHECK(bowTie.mask(4, 1) == 0);
}

TEST_CASE("applySelection: replace, add, subtract, intersect, each one step")
{
    Document document = blank(16, 16);
    CHECK(doc::applySelection(document, doc::rectShape(16, 16, {0, 0, 8, 16}), SelectMode::Replace, "Rect"));
    CHECK(countInside(document.selection()) == 128);
    CHECK(doc::applySelection(document, doc::rectShape(16, 16, {8, 0, 4, 16}), SelectMode::Add, "Add"));
    CHECK(countInside(document.selection()) == 192);
    CHECK(doc::applySelection(document, doc::rectShape(16, 16, {0, 0, 16, 8}), SelectMode::Subtract, "Sub"));
    CHECK(countInside(document.selection()) == 96);
    CHECK(doc::applySelection(document, doc::rectShape(16, 16, {0, 8, 4, 8}), SelectMode::Intersect, "Int"));
    CHECK(countInside(document.selection()) == 32);
    CHECK_FALSE(doc::applySelection(document, doc::rectShape(16, 16, {0, 8, 4, 8}), SelectMode::Add, "Again"));
    CHECK(document.history().size() == 4);
    document.undo();
    CHECK(countInside(document.selection()) == 96);
}

TEST_CASE("fillMask: paints inside the mask and the selection only")
{
    Document document = blank(8, 8);
    const doc::LayerId id = document.layer(0).id();
    doc::applySelection(document, doc::rectShape(8, 8, {0, 0, 4, 8}), SelectMode::Replace, "Left");
    const doc::Shape region = doc::rectShape(8, 8, {2, 2, 4, 4});
    CHECK(doc::fillMask(document, id, region.mask, region.bounds, {10, 20, 30, 255}, "Fill"));
    const Image<std::uint8_t> &px = document.layer(0).pixels();
    CHECK(px(3, 3, 0) == 10);
    CHECK(px(3, 3, 3) == 255);
    CHECK(px(4, 3, 0) == 0); // outside the selection
    CHECK(px(1, 3, 0) == 0); // outside the mask
    document.undo();
    CHECK(document.layer(0).pixels()(3, 3, 0) == 0);
}

TEST_CASE("Seeds: add, remove the nearest, clear, all undoable")
{
    Document document = blank(20, 20);
    doc::addSeed(document, {3, 3});
    doc::addSeed(document, {10, 10});
    doc::addSeed(document, {10, 10}); // already there: no step
    doc::addSeed(document, {25, 0});  // outside: no step
    CHECK(document.seeds().size() == 2);
    CHECK(document.history().size() == 2);
    CHECK_FALSE(doc::removeSeedNear(document, {16, 16}, 3));
    CHECK(doc::removeSeedNear(document, {9.6f, 11}, 3));
    CHECK(document.seeds() == std::vector<core::Vec2i>{{3, 3}});
    doc::clearSeeds(document);
    CHECK(document.seeds().empty());
    document.undo();
    document.undo();
    CHECK(document.seeds().size() == 2);
}
