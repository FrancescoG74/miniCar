#include <catch2/catch_all.hpp>

#include "TrackMath.h"

using track_math::wrapS;
using track_math::wrappedGap;

TEST_CASE("wrapS leaves values already inside range unchanged", "[track_math]") {
    REQUIRE(wrapS(50.0f, 100.0f) == Catch::Approx(50.0f));
    REQUIRE(wrapS(0.0f, 100.0f) == Catch::Approx(0.0f));
}

TEST_CASE("wrapS wraps values past the total length back into range", "[track_math]") {
    REQUIRE(wrapS(150.0f, 100.0f) == Catch::Approx(50.0f));
    REQUIRE(wrapS(250.0f, 100.0f) == Catch::Approx(50.0f));
}

TEST_CASE("wrapS wraps negative values into range", "[track_math]") {
    REQUIRE(wrapS(-10.0f, 100.0f) == Catch::Approx(90.0f));
    REQUIRE(wrapS(-150.0f, 100.0f) == Catch::Approx(50.0f));
}

TEST_CASE("wrapS result always lands between zero and totalLength", "[track_math]") {
    for (float s = -500.0f; s <= 500.0f; s += 17.0f) {
        float w = wrapS(s, 100.0f);
        REQUIRE(w >= 0.0f);
        REQUIRE(w < 100.0f);
    }
}

TEST_CASE("wrappedGap returns the raw difference when well within half length", "[track_math]") {
    REQUIRE(wrappedGap(60.0f, 50.0f, 100.0f) == Catch::Approx(10.0f));
    REQUIRE(wrappedGap(50.0f, 60.0f, 100.0f) == Catch::Approx(-10.0f));
}

TEST_CASE("wrappedGap wraps a gap that crosses the seam to the shorter signed path", "[track_math]") {
    // ahead=5, behind=95, totalLength=100: raw diff is -90, wrapped should be +10.
    REQUIRE(wrappedGap(5.0f, 95.0f, 100.0f) == Catch::Approx(10.0f));
    // ahead=95, behind=5: raw diff is +90, wrapped should be -10.
    REQUIRE(wrappedGap(95.0f, 5.0f, 100.0f) == Catch::Approx(-10.0f));
}

TEST_CASE("wrappedGap of a point with itself is zero", "[track_math]") {
    REQUIRE(wrappedGap(42.0f, 42.0f, 100.0f) == Catch::Approx(0.0f));
}

TEST_CASE("wrappedGap magnitude never exceeds half the total length", "[track_math]") {
    const float total = 100.0f;
    const float half = total / 2.0f;
    for (float a = 0.0f; a < total; a += 13.0f) {
        for (float b = 0.0f; b < total; b += 19.0f) {
            float gap = wrappedGap(a, b, total);
            REQUIRE(gap > -half - 1e-3f);
            REQUIRE(gap <= half + 1e-3f);
        }
    }
}
