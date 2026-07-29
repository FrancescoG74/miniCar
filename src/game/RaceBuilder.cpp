#include "game/RaceBuilder.h"

#include "Track.h"

RaceBuilder& RaceBuilder::withGrid(float laneOffset) {
    m_wantGrid = true;
    m_laneOffset = laneOffset;
    return *this;
}

RaceBuilder& RaceBuilder::withRocks() {
    m_wantRocks = true;
    return *this;
}

RaceBuilder& RaceBuilder::withGates() {
    m_wantGates = true;
    m_gateFractions.clear(); // signals "use Gate::createInitialGates defaults"
    return *this;
}

RaceBuilder& RaceBuilder::withGatesAtFractions(std::initializer_list<float> fractions) {
    m_wantGates = true;
    m_gateFractions.assign(fractions);
    return *this;
}

RaceBuilder::Race RaceBuilder::build(const Track& track, float trackWidth) const {
    Race race;

    if (m_wantGrid) {
        race.cars = Car::createInitialGrid(m_laneOffset);
    }

    if (m_wantRocks) {
        race.rocks = Rock::createInitialRocks(track);
    }

    if (m_wantGates) {
        if (m_gateFractions.empty()) {
            race.gates = Gate::createInitialGates(track, trackWidth);
        } else {
            const float L = track.totalLength();
            const float halfWidth = trackWidth / 2.0f;
            race.gates.reserve(m_gateFractions.size());
            for (float f : m_gateFractions) {
                race.gates.emplace_back(L * f, track, halfWidth);
            }
        }
    }

    return race;
}
