#include "game/TrackGenerator.h"

#include <cmath>
#include <vector>

#include "Track.h"

namespace {
constexpr float kPi = 3.14159265358979323846f;
constexpr std::size_t kControlPointCount = 32;

constexpr TrackGenerator::TrackType kTrackTypes[] = {
    { "Grand Oval", 330.0f, 190.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    { "Sawtooth Bay", 290.0f, 205.0f, 0.12f, 0.22f, 0.08f, 1.0f, 0.18f },
    { "Winding Ribbon", 350.0f, 165.0f, 0.28f, 0.08f, 0.09f, 2.0f, 0.13f },
    { "Three Point Crown", 300.0f, 190.0f, 0.08f, 0.29f, 0.06f, 0.5f, 0.08f },
    { "Crescent Bite", 340.0f, 215.0f, 0.24f, 0.14f, 0.04f, 2.6f, 0.16f },
    { "Twin Lungs", 360.0f, 160.0f, 0.31f, 0.06f, 0.05f, 1.4f, 0.10f },
    { "Crater Run", 285.0f, 220.0f, 0.15f, 0.20f, 0.11f, 3.2f, 0.18f },
    { "Serpent Loop", 350.0f, 190.0f, 0.19f, 0.14f, 0.13f, 4.0f, 0.16f },
    { "Spiral Shard", 315.0f, 215.0f, 0.27f, 0.12f, 0.04f, 5.0f, 0.12f },
    { "Moon Knot", 340.0f, 170.0f, 0.10f, 0.23f, 0.12f, 5.7f, 0.20f },
};

Track createTrack(const TrackGenerator::TrackType& type, float centerX, float centerY,
                  float trackWidth, float scale) {
    std::vector<SDL_FPoint> controlPoints;
    controlPoints.reserve(kControlPointCount);
    for (std::size_t index = 0; index < kControlPointCount; ++index) {
        const float angle = 2.0f * kPi * static_cast<float>(index) /
                            static_cast<float>(kControlPointCount);
        const float radial = 1.0f +
            type.lobeA * std::sin(2.0f * angle + type.phase) +
            type.lobeB * std::cos(3.0f * angle - type.phase) +
            type.lobeC * std::sin(5.0f * angle + 0.7f * type.phase);
        const float warpedAngle = angle + type.twist * std::sin(2.0f * angle + type.phase * 0.5f);
        controlPoints.push_back({
            centerX + type.radiusX * scale * radial * std::cos(warpedAngle),
            centerY + type.radiusY * scale * radial * std::sin(warpedAngle),
        });
    }
    return Track(centerX, centerY, trackWidth * scale, std::move(controlPoints));
}
}

const TrackGenerator::TrackType& TrackGenerator::typeAt(std::size_t trackIndex) {
    return kTrackTypes[trackIndex % kTrackCount];
}

Track TrackGenerator::createForRace(std::size_t raceNumber, float centerX, float centerY,
                                    float trackWidth) {
    const TrackType& type = typeAt(raceNumber);
    return createTrack(type, centerX, centerY, trackWidth, 1.0f);
}

Track TrackGenerator::createPreview(std::size_t trackIndex, float centerX, float centerY,
                                    float trackWidth, float scale) {
    const TrackType& type = typeAt(trackIndex);
    return createTrack(type, centerX, centerY, trackWidth, scale);
}
