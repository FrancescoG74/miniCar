#pragma once

#include <cstddef>

class Track;

class TrackGenerator {
public:
    static Track createForRace(std::size_t raceNumber, float centerX, float centerY,
                               float trackWidth);
};
