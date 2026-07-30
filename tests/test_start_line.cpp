#include <catch2/catch_all.hpp>
#include <cmath>

#include "Track.h"
#include "actor/StartLine.h"

namespace {
Track makeTrack() {
    return Track(0.0f, 0.0f, 200.0f, 50.0f, 40.0f);
}
} // namespace

TEST_CASE("StartLine defaults to s=0 and sits at the track's origin sample", "[startline]") {
    Track track = makeTrack();
    StartLine line(track, 40.0f);

    REQUIRE(line.s == Catch::Approx(0.0f));
    TrackPoint p = track.sample(0.0f);
    auto& pos = line.getPosition();
    REQUIRE(pos.x == Catch::Approx(p.x));
    REQUIRE(pos.y == Catch::Approx(p.y));
}

TEST_CASE("StartLine can be placed at an arbitrary arc-length position", "[startline]") {
    Track track = makeTrack();
    StartLine line(track, 40.0f, 75.0f);

    REQUIRE(line.s == Catch::Approx(75.0f));
    TrackPoint p = track.sample(75.0f);
    auto& pos = line.getPosition();
    REQUIRE(pos.x == Catch::Approx(p.x));
    REQUIRE(pos.y == Catch::Approx(p.y));
}

TEST_CASE("StartLine stores the track width it was built with", "[startline]") {
    Track track = makeTrack();
    StartLine line(track, 40.0f);
    REQUIRE(line.trackWidth == Catch::Approx(40.0f));
}
