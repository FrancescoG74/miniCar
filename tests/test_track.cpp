#include <catch2/catch_all.hpp>
#include <cmath>

#include "Track.h"
#include "game/TrackGenerator.h"

namespace {
constexpr float kPi = 3.14159265358979323846f;
}

TEST_CASE("TrackGenerator alternates two reproducible race tracks", "[track]") {
    const Track first = TrackGenerator::createForRace(0, 800.0f, 450.0f, 130.0f);
    const Track second = TrackGenerator::createForRace(1, 800.0f, 450.0f, 130.0f);
    const Track third = TrackGenerator::createForRace(2, 800.0f, 450.0f, 130.0f);
    const Track fourth = TrackGenerator::createForRace(3, 800.0f, 450.0f, 130.0f);

    REQUIRE(first.totalLength() != Catch::Approx(second.totalLength()));
    REQUIRE(first.totalLength() == Catch::Approx(third.totalLength()));
    REQUIRE(second.totalLength() == Catch::Approx(fourth.totalLength()));
}

TEST_CASE("Track total length matches two straights plus two semicircles", "[track]") {
    Track track(0.0f, 0.0f, 200.0f, 50.0f, 40.0f);
    float expected = 2.0f * 200.0f + 2.0f * kPi * 50.0f;
    REQUIRE(track.totalLength() == Catch::Approx(expected));
}

TEST_CASE("Track sample at s=0 sits at the start of the top straight", "[track]") {
    Track track(0.0f, 0.0f, 200.0f, 50.0f, 40.0f);
    TrackPoint p = track.sample(0.0f);
    REQUIRE(p.x == Catch::Approx(-100.0f));
    REQUIRE(p.y == Catch::Approx(-50.0f));
    REQUIRE(p.angle == Catch::Approx(0.0f));
}

TEST_CASE("Track sample midway along the top straight moves only in x", "[track]") {
    Track track(0.0f, 0.0f, 200.0f, 50.0f, 40.0f);
    TrackPoint p = track.sample(100.0f);
    REQUIRE(p.x == Catch::Approx(0.0f));
    REQUIRE(p.y == Catch::Approx(-50.0f));
    REQUIRE(p.angle == Catch::Approx(0.0f));
}

TEST_CASE("Track sample negative s wraps to the equivalent positive position", "[track]") {
    Track track(0.0f, 0.0f, 200.0f, 50.0f, 40.0f);
    TrackPoint atNegative = track.sample(-1.0f);
    TrackPoint atWrapped = track.sample(track.totalLength() - 1.0f);
    REQUIRE(atNegative.x == Catch::Approx(atWrapped.x));
    REQUIRE(atNegative.y == Catch::Approx(atWrapped.y));
}

TEST_CASE("Track sample beyond total length wraps back to the start", "[track]") {
    Track track(0.0f, 0.0f, 200.0f, 50.0f, 40.0f);
    TrackPoint atZero = track.sample(0.0f);
    TrackPoint atWrapped = track.sample(track.totalLength() + 5.0f);
    TrackPoint atFive = track.sample(5.0f);
    REQUIRE(atWrapped.x == Catch::Approx(atFive.x));
    REQUIRE(atWrapped.y == Catch::Approx(atFive.y));
    (void)atZero;
}

TEST_CASE("Track sample stays at constant radius from the curve center through the first curve", "[track]") {
    Track track(0.0f, 0.0f, 200.0f, 50.0f, 40.0f);
    // The right-hand semicircle starts right after the top straight (s == straightLength)
    // and is centered on (centerX + straightLength/2, centerY) == (100, 0).
    const float curveCenterX = 100.0f;
    const float curveCenterY = 0.0f;
    for (float offset = 1.0f; offset < 50.0f * kPi; offset += 10.0f) {
        TrackPoint p = track.sample(200.0f + offset);
        float dx = p.x - curveCenterX;
        float dy = p.y - curveCenterY;
        float dist = std::sqrt(dx * dx + dy * dy);
        REQUIRE(dist == Catch::Approx(50.0f).margin(0.01f));
    }
}
