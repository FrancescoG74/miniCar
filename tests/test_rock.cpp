#include <catch2/catch_all.hpp>
#include <cmath>

#include "Track.h"
#include "actor/Rock.h"

namespace {
Track makeTrack() {
    return Track(0.0f, 0.0f, 200.0f, 50.0f, 40.0f);
}
} // namespace

TEST_CASE("Rock world position matches the track sample offset by its lane", "[rock]") {
    Track track = makeTrack();
    Rock rock(50.0f, 10.0f, track, 15.0f);

    TrackPoint p = track.sample(50.0f);
    float perpX = -std::sin(p.angle);
    float perpY = std::cos(p.angle);
    auto& pos = rock.getPosition();
    REQUIRE(pos.x == Catch::Approx(p.x + perpX * 10.0f));
    REQUIRE(pos.y == Catch::Approx(p.y + perpY * 10.0f));
}

TEST_CASE("Rock stores its arc-length, lane offset and size", "[rock]") {
    Track track = makeTrack();
    Rock rock(50.0f, -20.0f, track, 18.0f);
    REQUIRE(rock.s == Catch::Approx(50.0f));
    REQUIRE(rock.laneOffset == Catch::Approx(-20.0f));
    REQUIRE(rock.size == Catch::Approx(18.0f));
}

TEST_CASE("Rock::createInitialRocks produces a plausible count of rocks", "[rock]") {
    Track track = makeTrack();
    auto rocks = Rock::createInitialRocks(track);
    REQUIRE(rocks.size() >= 5);
    REQUIRE(rocks.size() <= 8);
}

TEST_CASE("Rock::createInitialRocks keeps every rock outside the AI lanes", "[rock]") {
    Track track = makeTrack();
    auto rocks = Rock::createInitialRocks(track);
    for (const auto& rock : rocks) {
        REQUIRE(std::abs(rock.laneOffset) >= 45.0f);
        REQUIRE(std::abs(rock.laneOffset) <= 50.0f);
        REQUIRE(rock.size >= 12.0f);
        REQUIRE(rock.size <= 22.0f);
    }
}
