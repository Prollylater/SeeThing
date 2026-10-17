#include <doctest/doctest.h>

#include "ui/CanvasView.h"

using core::Vec2f;
using ui::CanvasView;

namespace
{
    void checkNear(Vec2f got, Vec2f want)
    {
        CHECK(got.x == doctest::Approx(want.x).epsilon(1e-5));
        CHECK(got.y == doctest::Approx(want.y).epsilon(1e-5));
    }
}

TEST_CASE("CanvasView: toImage inverts toScreen")
{
    CanvasView v;
    v.pan = {13, -7};
    v.zoom = 2.5f;
    const Vec2f origin{100, 50};
    checkNear(v.toScreen({0, 0}, origin), {113, 43});
    checkNear(v.toScreen({4, 2}, origin), {123, 48});
    for (Vec2f p : {Vec2f{0, 0}, Vec2f{3.5f, 9}, Vec2f{-2, 40}})
        checkNear(v.toImage(v.toScreen(p, origin), origin), p);
}

TEST_CASE("CanvasView: zooming keeps the point under the cursor")
{
    CanvasView v;
    v.pan = {20, 30};
    const Vec2f origin{10, 10}, cursor{210, 140};
    const Vec2f before = v.toImage(cursor, origin);
    v.zoomAround(cursor, origin, 4.0f);
    CHECK(v.zoom == 4.0f);
    checkNear(v.toImage(cursor, origin), before);

    v.zoomAround(cursor, origin, 1000.0f);
    CHECK(v.zoom == CanvasView::maxZoom);
    checkNear(v.toImage(cursor, origin), before);
}

TEST_CASE("CanvasView: fit shows the whole image, centered, never enlarged")
{
    CanvasView v;
    v.fit({1000, 500}, {500, 500});
    CHECK(v.zoom == 0.5f);
    checkNear(v.pan, {0, 125});
    CHECK(v.fittedSize == Vec2f{1000, 500});

    v.fit({100, 50}, {500, 500});
    CHECK(v.zoom == 1.0f);
    checkNear(v.pan, {200, 225});

    v.zoom = 3;
    v.actualSize({100, 50}, {500, 500});
    CHECK(v.zoom == 1.0f);
    checkNear(v.pan, {200, 225});
}

TEST_CASE("CanvasView: wheel notches reach 100% and powers of two exactly")
{
    CHECK(CanvasView::stepZoom(1.0f, 4) == 2.0f);
    CHECK(CanvasView::stepZoom(2.0f, -4) == 1.0f);

    float zoom = 0.37f;
    bool hitOne = false;
    for (int i = 0; i < 8 && !hitOne; ++i)
    {
        zoom = CanvasView::stepZoom(zoom, 1);
        hitOne = zoom == 1.0f;
    }
    CHECK(hitOne);

    CHECK(CanvasView::stepZoom(CanvasView::maxZoom, 3) == CanvasView::maxZoom);
    CHECK(CanvasView::stepZoom(CanvasView::minZoom, -3) == CanvasView::minZoom);
    // Touchpads send fractions: they must still move
    CHECK(CanvasView::stepZoom(1.0f, 0.2f) > 1.0f);
}
