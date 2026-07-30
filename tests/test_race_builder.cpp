#include <catch2/catch_all.hpp>

#include "Track.h"
#include "game/RaceBuilder.h"

namespace {
Track makeTrack() {
    return Track(0.0f, 0.0f, 200.0f, 50.0f, 40.0f);
}
} // namespace

TEST_CASE("RaceBuilder with nothing requested builds an empty race", "[race_builder]") {
    Track track = makeTrack();
    auto race = RaceBuilder{}.build(track, 40.0f);
    REQUIRE(race.cars.empty());
    REQUIRE(race.rocks.empty());
    REQUIRE(race.gates.empty());
}

TEST_CASE("RaceBuilder::withGrid builds the fixed six-car grid", "[race_builder]") {
    Track track = makeTrack();
    auto race = RaceBuilder{}.withGrid(25.0f).build(track, 40.0f);
    REQUIRE(race.cars.size() == 6);
    REQUIRE(race.rocks.empty());
    REQUIRE(race.gates.empty());
}

TEST_CASE("RaceBuilder::withRocks builds a non-empty rock field", "[race_builder]") {
    Track track = makeTrack();
    auto race = RaceBuilder{}.withRocks().build(track, 40.0f);
    REQUIRE_FALSE(race.rocks.empty());
    REQUIRE(race.cars.empty());
}

TEST_CASE("RaceBuilder::withGates uses the default 25%/75% positions", "[race_builder]") {
    Track track = makeTrack();
    auto race = RaceBuilder{}.withGates().build(track, 40.0f);
    REQUIRE(race.gates.size() == 2);
    REQUIRE(race.gates[0].s == Catch::Approx(track.totalLength() * 0.25f));
    REQUIRE(race.gates[1].s == Catch::Approx(track.totalLength() * 0.75f));
}

TEST_CASE("RaceBuilder::withGatesAtFractions places gates at custom fractions", "[race_builder]") {
    Track track = makeTrack();
    auto race = RaceBuilder{}.withGatesAtFractions({ 0.1f, 0.5f, 0.9f }).build(track, 40.0f);
    REQUIRE(race.gates.size() == 3);
    REQUIRE(race.gates[0].s == Catch::Approx(track.totalLength() * 0.1f));
    REQUIRE(race.gates[1].s == Catch::Approx(track.totalLength() * 0.5f));
    REQUIRE(race.gates[2].s == Catch::Approx(track.totalLength() * 0.9f));
}

TEST_CASE("RaceBuilder combines grid, rocks and gates together", "[race_builder]") {
    Track track = makeTrack();
    auto race = RaceBuilder{}
        .withGrid(25.0f)
        .withRocks()
        .withGates()
        .build(track, 40.0f);
    REQUIRE(race.cars.size() == 6);
    REQUIRE_FALSE(race.rocks.empty());
    REQUIRE(race.gates.size() == 2);
}
