#include "game/RaceSession.h"

#include "CollisionSystem.h"
#include "Track.h"
#include "game/DriverAssignmentService.h"
#include "game/RaceBuilder.h"

RaceSession::RaceSession(const Track& track, float trackWidth, RaceTuning tuning, AiTuning aiTuning)
    : m_track(track), m_trackWidth(trackWidth), m_tuning(tuning), m_aiTuning(aiTuning) {}

void RaceSession::reset() {
    auto race = RaceBuilder{}
        .withGrid(m_aiTuning.homeLaneOffset)
        .withRocks()
        .withGates()
        .build(m_track, m_trackWidth);
    m_cars = std::move(race.cars);
    m_rocks = std::move(race.rocks);
    m_gates = std::move(race.gates);

    m_player1Index = DriverAssignmentService::assignPlayer1(m_cars);
    m_player2Index.reset();
    m_winnerIndex.reset();
}

void RaceSession::update(float dt, const Uint8* keys) {
    for (auto& gate : m_gates) gate.update(dt);

    std::vector<float> blockedS;
    for (const auto& gate : m_gates) {
        if (gate.isClosed()) blockedS.push_back(gate.s);
    }

    CarControls controls;
    controls.keys = keys;
    controls.maxSpeed = m_tuning.maxCarSpeed;
    controls.playerAccel = m_tuning.playerAcceleration;
    controls.playerBrake = m_tuning.playerBrake;
    controls.playerSteerRate = m_tuning.playerSteerRate;
    controls.laneLimit = m_tuning.laneLimit;
    controls.aiAccel = m_tuning.aiAcceleration;
    controls.recoveryBoost = m_tuning.recoveryBoost;
    controls.blockedSPositions = &blockedS;
    controls.track = &m_track;
    controls.aiTuning = &m_aiTuning;

    const float totalLength = m_track.totalLength();
    for (std::size_t index = 0; index < m_cars.size(); ++index) {
        Car& car = m_cars[index];
        car.update(dt, m_cars, index, totalLength, controls);
        if (car.laps >= m_tuning.lapsToWin && !m_winnerIndex) {
            m_winnerIndex = index;
        }
    }

    CollisionSystem::Config collisionConfig{
        totalLength,
        m_tuning.carLength,
        m_tuning.carWidth,
    };
    CollisionSystem::resolveAll(m_cars, m_rocks, m_gates, collisionConfig);
}

std::optional<std::size_t> RaceSession::joinPlayer2() {
    if (!m_player1Index || m_player2Index || m_winnerIndex) return std::nullopt;
    m_player2Index = DriverAssignmentService::assignPlayer2(m_cars, *m_player1Index);
    return m_player2Index;
}

bool RaceSession::removePlayer2() {
    if (!m_player2Index || m_winnerIndex) return false;
    if (!DriverAssignmentService::removePlayer2(m_cars, *m_player2Index)) return false;
    m_player2Index.reset();
    return true;
}
