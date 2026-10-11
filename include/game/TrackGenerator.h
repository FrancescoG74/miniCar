#pragma once

#include <cstddef>

class Track;

class TrackGenerator {
public:
    struct TrackType {
        const char* name;
        float radiusX;
        float radiusY;
        float lobeA;
        float lobeB;
        float lobeC;
        float phase;
        float twist;
    };

    static constexpr std::size_t kTrackCount = 10;

    static const TrackType& typeAt(std::size_t trackIndex);
    static Track createForRace(std::size_t raceNumber, float centerX, float centerY,
                               float trackWidth);
    static Track createPreview(std::size_t trackIndex, float centerX, float centerY,
                               float trackWidth, float scale);
};
