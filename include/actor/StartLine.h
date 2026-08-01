#pragma once

#include <SDL3/SDL.h>

#include "actor/Actor.h"

class Track;

// The checkered start/finish stripe drawn across the circuit. Its world-space
// orientation is sampled from the track at construction so the render path stays
// a plain per-frame draw with no track lookups.
class StartLine : public Actor {
public:
    static constexpr int kCheckerCount = 8;
    static constexpr float kThickness = 6.0f;

    float s = 0.0f;              // arc-length position on the track centerline
    float trackWidth = 0.0f;     // total drivable width; sets the line's length

    StartLine(const Track& track, float trackWidth, float s = 0.0f);

    void render(SDL_Renderer* renderer) const override;

private:
    // Precomputed unit vectors at `s`: forward = along track, perp = across track.
    float forwardX = 0.0f;
    float forwardY = 0.0f;
    float perpX = 0.0f;
    float perpY = 0.0f;
};
