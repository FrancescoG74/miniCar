#pragma once

#include <initializer_list>
#include <vector>

#include "actor/Car.h"
#include "actor/Gate.h"
#include "actor/Rock.h"

class Track;

// Builder pattern that consolidates the three per-race factories
// (Car::createInitialGrid, Rock::createInitialRocks, Gate::createInitialGates)
// behind a single fluent API. Callers describe *what* the race should look
// like, then call build(track, trackWidth) once to materialize it.
//
// Usage:
//     auto race = RaceBuilder{}
//         .withGrid(kLaneOffset)
//         .withRocks()
//         .withGates()               // default: two gates at 25% and 75% of the loop
//         .build(*track, kTrackWidth);
//     cars  = std::move(race.cars);
//     rocks = std::move(race.rocks);
//     gates = std::move(race.gates);
//
// Cars are move-only (they own an InputController), so `Race` is move-only too.
class RaceBuilder {
public:
    struct Race {
        std::vector<Car> cars;
        std::vector<Rock> rocks;
        std::vector<Gate> gates;

        Race() = default;
        Race(const Race&) = delete;
        Race& operator=(const Race&) = delete;
        Race(Race&&) noexcept = default;
        Race& operator=(Race&&) noexcept = default;
    };

    // Include the fixed 6-car grid (staggered rows of two lanes at ±laneOffset).
    RaceBuilder& withGrid(float laneOffset);

    // Include the randomly-scattered rocks.
    RaceBuilder& withRocks();

    // Include the default gates (two at 25% and 75% of the circuit).
    RaceBuilder& withGates();

    // Include gates at custom arc-length fractions of the total loop length.
    RaceBuilder& withGatesAtFractions(std::initializer_list<float> fractions);

    // Materializes everything requested. `track` and `trackWidth` come from the
    // Game; the same values already used to build the visible track.
    Race build(const Track& track, float trackWidth) const;

private:
    bool m_wantGrid = false;
    float m_laneOffset = 0.0f;
    bool m_wantRocks = false;
    bool m_wantGates = false;
    std::vector<float> m_gateFractions;
};
