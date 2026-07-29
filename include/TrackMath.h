#pragma once

#include <cmath>

// Shared arc-length math for the closed stadium circuit. Extracted so
// Car::update, CollisionSystem and any future track-aware code stop
// re-implementing the same wrap logic in slightly different ways.
namespace track_math {

// Signed distance from `behind` to `ahead` along the closed circuit, wrapped
// into (-totalLength/2, totalLength/2]. Positive when `ahead` really is ahead
// (in the direction of travel), negative when it's behind. Handles the case
// where the raw difference crosses the start/finish seam.
inline float wrappedGap(float ahead, float behind, float totalLength) {
    float gap = ahead - behind;
    const float half = totalLength / 2.0f;
    while (gap > half) gap -= totalLength;
    while (gap < -half) gap += totalLength;
    return gap;
}

// Wraps an arc-length parameter into [0, totalLength).
inline float wrapS(float s, float totalLength) {
    float w = std::fmod(s, totalLength);
    if (w < 0.0f) w += totalLength;
    return w;
}

} // namespace track_math
