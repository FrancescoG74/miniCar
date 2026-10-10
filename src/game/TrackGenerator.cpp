#include "game/TrackGenerator.h"

#include "Track.h"

Track TrackGenerator::createForRace(std::size_t raceNumber, float centerX, float centerY,
                                    float trackWidth) {
    constexpr float kTrackStraightLengths[] = { 500.0f, 350.0f };
    constexpr float kTrackRadii[] = { 190.0f, 155.0f };
    constexpr std::size_t kTrackCount = sizeof(kTrackStraightLengths) / sizeof(kTrackStraightLengths[0]);
    const std::size_t trackIndex = raceNumber % kTrackCount;

    return Track(centerX, centerY, kTrackStraightLengths[trackIndex], kTrackRadii[trackIndex],
                 trackWidth);
}
