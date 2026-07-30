#include <catch2/catch_all.hpp>

#include "Track.h"
#include "game/RaceSession.h"
#include "game/RaceTuning.h"

namespace {
Track makeTrack() {
    return Track(0.0f, 0.0f, 200.0f, 50.0f, 40.0f);
}
} // namespace

TEST_CASE("RaceSession::reset populates cars/rocks/gates and assigns Player1", "[race_session]") {
    Track track = makeTrack();
    RaceSession session(track, 40.0f, RaceTuning{}, AiTuning{}, 1);
    session.reset();

    REQUIRE(session.cars().size() == 6);
    REQUIRE_FALSE(session.rocks().empty());
    REQUIRE(session.gates().size() == 2);
    REQUIRE(session.player1Index().has_value());
    REQUIRE_FALSE(session.player2Index().has_value());
    REQUIRE_FALSE(session.winnerIndex().has_value());
}

TEST_CASE("RaceSession::update moves at least one car forward", "[race_session]") {
    Track track = makeTrack();
    RaceSession session(track, 40.0f, RaceTuning{}, AiTuning{}, 2);
    session.reset();

    float totalDistanceBefore = 0.0f;
    for (const auto& car : session.cars()) totalDistanceBefore += car.distanceTraveled;

    session.update(0.1f, nullptr);

    float totalDistanceAfter = 0.0f;
    for (const auto& car : session.cars()) totalDistanceAfter += car.distanceTraveled;

    REQUIRE(totalDistanceAfter > totalDistanceBefore);
}

TEST_CASE("RaceSession::joinPlayer2 assigns a different car than Player1", "[race_session]") {
    Track track = makeTrack();
    RaceSession session(track, 40.0f, RaceTuning{}, AiTuning{}, 3);
    session.reset();

    auto p2 = session.joinPlayer2();
    REQUIRE(p2.has_value());
    REQUIRE(*p2 != *session.player1Index());
    REQUIRE(session.player2Index() == p2);
}

TEST_CASE("RaceSession::joinPlayer2 is a no-op once Player2 already joined", "[race_session]") {
    Track track = makeTrack();
    RaceSession session(track, 40.0f, RaceTuning{}, AiTuning{}, 4);
    session.reset();

    REQUIRE(session.joinPlayer2().has_value());
    auto second = session.joinPlayer2();
    REQUIRE_FALSE(second.has_value());
}

TEST_CASE("RaceSession::removePlayer2 clears Player2 and can be called again safely", "[race_session]") {
    Track track = makeTrack();
    RaceSession session(track, 40.0f, RaceTuning{}, AiTuning{}, 5);
    session.reset();
    session.joinPlayer2();

    REQUIRE(session.removePlayer2());
    REQUIRE_FALSE(session.player2Index().has_value());
    REQUIRE_FALSE(session.removePlayer2());
}

TEST_CASE("RaceSession::tuning reflects the values passed at construction", "[race_session]") {
    Track track = makeTrack();
    RaceTuning tuning;
    tuning.lapsToWin = 3;
    tuning.maxCarSpeed = 200.0f;
    RaceSession session(track, 40.0f, tuning, AiTuning{}, 6);

    REQUIRE(session.tuning().lapsToWin == 3);
    REQUIRE(session.tuning().maxCarSpeed == Catch::Approx(200.0f));
}

TEST_CASE("RaceSession declares a winner once a car completes enough laps", "[race_session]") {
    Track track = makeTrack();
    RaceTuning tuning;
    tuning.lapsToWin = 1;
    RaceSession session(track, 40.0f, tuning, AiTuning{}, 7);
    session.reset();

    bool wonWithinBudget = false;
    for (int i = 0; i < 5000 && !wonWithinBudget; ++i) {
        session.update(0.05f, nullptr);
        wonWithinBudget = session.winnerIndex().has_value();
    }

    REQUIRE(wonWithinBudget);
}

TEST_CASE("RaceSession freezes player assignment after a winner is declared", "[race_session]") {
    Track track = makeTrack();
    RaceTuning tuning;
    tuning.lapsToWin = 0;
    RaceSession session(track, 40.0f, tuning, AiTuning{}, 8);
    session.reset();

    session.update(0.1f, nullptr);

    REQUIRE(session.winnerIndex().has_value());
    REQUIRE_FALSE(session.joinPlayer2().has_value());
    REQUIRE_FALSE(session.removePlayer2());
}
