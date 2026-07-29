#pragma once

#include <SDL2/SDL.h>
#include <vector>

#include "actor/Actor.h"

class Car;
class Track;

// A gate barrier across the track that cyclically opens and closes. When closed it
// blocks cars from passing (they collide with it like they would a wall). Gates are
// positioned via an arc-length `s` on the track centerline and pre-compute their
// world-space post positions at construction so per-frame rendering stays cheap.
class Gate : public Actor {
public:
    // Complete open+close cycle length, and how much of it the gate spends closed.
    static constexpr float kCycleDuration = 8.0f;
    static constexpr float kClosedDuration = 4.0f;

    float s = 0.0f;              // arc-length position on the track centerline
    float trackHalfWidth = 0.0f; // half of the track's drivable width
    float thickness = 8.0f;      // forward thickness (for drawing + collision)
    float phaseOffset = 0.0f;    // shifts this gate's cycle relative to the global clock
    float elapsed = 0.0f;

    // Precomputed world-space geometry sampled from the track at construction.
    SDL_FPoint postA{ 0.0f, 0.0f };
    SDL_FPoint postB{ 0.0f, 0.0f };
    float perpX = 0.0f;      // across-track unit vector (postA points along +perp)
    float perpY = 0.0f;
    float forwardX = 0.0f;   // along-track unit vector at this s
    float forwardY = 0.0f;

    Gate(float s, float phaseOffset, const Track& track, float trackHalfWidth);

    void update(float dt);
    void render(SDL_Renderer* renderer) const override;

    // True while the gate is currently blocking cars from passing.
    bool isClosed() const;

    // Two gates placed on opposite sides of the circuit with staggered phases so
    // that (typically) one is open while the other is closed.
    static std::vector<Gate> createInitialGates(const Track& track, float trackWidth);

    // Push any car overlapping a *closed* gate back along the track, zeroing its
    // speed. Open gates are ignored.
    static void resolveCarCollisions(std::vector<Car>& cars,
                                       const std::vector<Gate>& gates,
                                       float totalLength,
                                       float carLength);
};
