#include <catch2/catch_all.hpp>
#include <cmath>

#include "Track.h"
#include "actor/Gate.h"

namespace {
Track makeTrack() {
    return Track(0.0f, 0.0f, 200.0f, 50.0f, 40.0f);
}

// Gate's initial open/closed phase is randomized 50/50; retry construction a
// bounded number of times to deterministically exercise each branch.
Gate makeGateWithState(const Track& track, bool wantClosed) {
    for (int attempt = 0; attempt < 100; ++attempt) {
        Gate gate(track.totalLength() * 0.25f, track, 20.0f);
        if (gate.isClosed() == wantClosed) return gate;
    }
    FAIL("could not roll a gate in the requested state after 100 attempts");
    return Gate(0.0f, track, 20.0f);
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

TEST_CASE("Gate can be rolled into a closed initial state", "[gate]") {
    Track track = makeTrack();
    Gate gate = makeGateWithState(track, true);
    REQUIRE(gate.isClosed());
}

TEST_CASE("Gate can be rolled into an open initial state", "[gate]") {
    Track track = makeTrack();
    Gate gate = makeGateWithState(track, false);
    REQUIRE_FALSE(gate.isClosed());
}

TEST_CASE("Gate toggles open/closed state over enough elapsed time", "[gate]") {
    Track track = makeTrack();
    Gate gate(10.0f, track, 20.0f);
    bool initial = gate.isClosed();

    bool sawToggle = false;
    for (int i = 0; i < 200; ++i) {
        gate.update(0.25f);
        if (gate.isClosed() != initial) {
            sawToggle = true;
            break;
        }
    }
    REQUIRE(sawToggle);
}

TEST_CASE("Gate::createInitialGates places two gates at 25% and 75% of the loop", "[gate]") {
    Track track = makeTrack();
    auto gates = Gate::createInitialGates(track, 40.0f);
    REQUIRE(gates.size() == 2);
    REQUIRE(gates[0].s == Catch::Approx(track.totalLength() * 0.25f));
    REQUIRE(gates[1].s == Catch::Approx(track.totalLength() * 0.75f));
}
