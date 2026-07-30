#include <catch2/catch_all.hpp>
#include <cmath>
#include <random>

#include "Track.h"
#include "actor/Gate.h"

namespace {
Track makeTrack() {
    return Track(0.0f, 0.0f, 200.0f, 50.0f, 40.0f);
}

} // namespace

TEST_CASE("Gate posts are placed trackHalfWidth apart from the centerline sample", "[gate]") {
    Track track = makeTrack();
    Gate gate(10.0f, track, 20.0f);

    auto& pos = gate.getPosition();
    float distA = std::hypot(gate.postA.x - pos.x, gate.postA.y - pos.y);
    float distB = std::hypot(gate.postB.x - pos.x, gate.postB.y - pos.y);
    REQUIRE(distA == Catch::Approx(20.0f));
    REQUIRE(distB == Catch::Approx(20.0f));
}

TEST_CASE("Gate posts sit on opposite sides of the centerline", "[gate]") {
    Track track = makeTrack();
    Gate gate(10.0f, track, 20.0f);

    auto& pos = gate.getPosition();
    float midX = (gate.postA.x + gate.postB.x) / 2.0f;
    float midY = (gate.postA.y + gate.postB.y) / 2.0f;
    REQUIRE(midX == Catch::Approx(pos.x).margin(0.01f));
    REQUIRE(midY == Catch::Approx(pos.y).margin(0.01f));
}

TEST_CASE("Gate has reproducible phase changes with a fixed seed", "[gate]") {
    Track track = makeTrack();
    std::mt19937 rngA(1234);
    std::mt19937 rngB(1234);
    Gate gateA(10.0f, track, 20.0f, rngA);
    Gate gateB(10.0f, track, 20.0f, rngB);

    REQUIRE(gateA.isClosed() == gateB.isClosed());
    for (int i = 0; i < 100; ++i) {
        gateA.update(0.25f);
        gateB.update(0.25f);
        REQUIRE(gateA.isClosed() == gateB.isClosed());
    }
}

TEST_CASE("Gate state can be explicitly controlled", "[gate]") {
    Track track = makeTrack();
    std::mt19937 rng(9876);
    Gate gate(10.0f, track, 20.0f, rng);
    gate.setClosed(true);
    REQUIRE(gate.isClosed());
    gate.setClosed(false);
    REQUIRE_FALSE(gate.isClosed());
}

TEST_CASE("Gate::createInitialGates places two gates at 25% and 75% of the loop", "[gate]") {
    Track track = makeTrack();
    auto gates = Gate::createInitialGates(track, 40.0f);
    REQUIRE(gates.size() == 2);
    REQUIRE(gates[0].s == Catch::Approx(track.totalLength() * 0.25f));
    REQUIRE(gates[1].s == Catch::Approx(track.totalLength() * 0.75f));
}
