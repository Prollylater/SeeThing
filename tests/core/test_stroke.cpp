#include <doctest/doctest.h>

#include "core/Stroke.h"

#include <cmath>

using core::Vec2f;

TEST_CASE("DabSpacer: even spacing across segment boundaries")
{
    core::DabSpacer spacer;
    std::vector<Vec2f> dabs;
    spacer.begin({0, 0}, dabs);
    // Uneven segments: 7, 3, 0, 12 units along x
    for (float x : {7.0f, 10.0f, 10.0f, 22.0f})
        spacer.moveTo({x, 0}, 5.0f, dabs);
    REQUIRE(dabs.size() == 5); // 0, 5, 10, 15, 20
    for (std::size_t i = 0; i < dabs.size(); ++i)
    {
        CHECK(dabs[i].x == doctest::Approx(5.0f * float(i)));
        CHECK(dabs[i].y == 0.0f);
    }
}

TEST_CASE("DabSpacer: a long jump in one frame is filled, not dotted")
{
    core::DabSpacer spacer;
    std::vector<Vec2f> dabs;
    spacer.begin({10, 10}, dabs);
    spacer.moveTo({10 + 300, 10 + 400}, 2.0f, dabs); // 500 units in one frame
    CHECK(dabs.size() == 251);
    for (std::size_t i = 1; i < dabs.size(); ++i)
    {
        const Vec2f step = dabs[i] - dabs[i - 1];
        CHECK(std::hypot(step.x, step.y) == doctest::Approx(2.0f).epsilon(1e-3));
    }
}

TEST_CASE("smoothTowards: 0 follows, more trails")
{
    CHECK(core::smoothTowards({0, 0}, {10, 0}, 0.0f).x == doctest::Approx(10.0f));
    CHECK(core::smoothTowards({0, 0}, {10, 0}, 0.5f).x == doctest::Approx(5.0f));
    CHECK(core::smoothTowards({0, 0}, {10, 0}, 2.0f).x > 0.0f); // clamped, still moves
}
