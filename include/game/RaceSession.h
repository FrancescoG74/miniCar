#pragma once

#include <cstddef>
#include <optional>
#include <random>
#include <vector>

#include "game/RaceTuning.h"
#include "actor/Car.h"
#include "actor/Gate.h"
#include "actor/Rock.h"

class Track;

// Domain model for one race. It owns mutable world state and applies all
// simulation rules, while Game remains responsible for SDL events, audio,
// text textures and rendering.
class RaceSession {
public:
    RaceSession(const Track& track, float trackWidth, RaceTuning tuning, AiTuning aiTuning,
                std::uint32_t seed = std::random_device{}());

    // initialPlayers selects how many human drivers (0-2) are assigned before
    // the race starts; extra slots stay AI-controlled until joinPlayer2().
    void reset(int initialPlayers = 1);
    void update(float dt, const Uint8* keys);

    std::optional<std::size_t> joinPlayer2();
    bool removePlayer2();

    const RaceTuning& tuning() const { return m_tuning; }
    // Applied immediately; takes effect on the next lap-count check in update().
    void setLapsToWin(int laps) { m_tuning.lapsToWin = laps; }
    const std::optional<std::size_t>& player1Index() const { return m_player1Index; }
    const std::optional<std::size_t>& player2Index() const { return m_player2Index; }
    const std::optional<std::size_t>& winnerIndex() const { return m_winnerIndex; }

    std::vector<Car>& cars() { return m_cars; }
    const std::vector<Car>& cars() const { return m_cars; }
    const std::vector<Rock>& rocks() const { return m_rocks; }
    const std::vector<Gate>& gates() const { return m_gates; }

private:
    const Track& m_track;
    float m_trackWidth;
    RaceTuning m_tuning;
    AiTuning m_aiTuning;
    std::vector<Car> m_cars;
    std::vector<Rock> m_rocks;
    std::vector<Gate> m_gates;
    std::optional<std::size_t> m_player1Index;
    std::optional<std::size_t> m_player2Index;
    std::optional<std::size_t> m_winnerIndex;
    std::mt19937 m_rng;
};
