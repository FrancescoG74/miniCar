#pragma once

#include <SDL3/SDL.h>
#include <random>
#include <vector>

#include "actor/Actor.h"

class Track;

// A gate barrier across the track that cyclically opens and closes. When closed it
// blocks cars from passing (they collide with it like they would a wall). Gates are
// positioned via an arc-length `s` on the track centerline and pre-compute their
// world-space post positions at construction so per-frame rendering stays cheap.
class Gate : public Actor {
public:
    // Each gate independently rolls a new random duration every time it toggles
    // open/closed, within these bounds, so gates don't settle into a predictable
    // synchronized rhythm.
    static constexpr float kMinClosedDuration = 2.0f;
    static constexpr float kMaxClosedDuration = 5.0f;
    static constexpr float kMinOpenDuration = 2.5f;
    static constexpr float kMaxOpenDuration = 6.0f;

    float s = 0.0f;              // arc-length position on the track centerline
    float trackHalfWidth = 0.0f; // half of the track's drivable width
    float thickness = 8.0f;      // forward thickness (for drawing + collision)

    // Precomputed world-space geometry sampled from the track at construction.
    SDL_FPoint postA{ 0.0f, 0.0f };
    SDL_FPoint postB{ 0.0f, 0.0f };
    float perpX = 0.0f;      // across-track unit vector (postA points along +perp)
    float perpY = 0.0f;
    float forwardX = 0.0f;   // along-track unit vector at this s
    float forwardY = 0.0f;

    Gate(float s, const Track& track, float trackHalfWidth);
    Gate(float s, const Track& track, float trackHalfWidth, std::mt19937& rng);

    void update(float dt);
    void render(SDL_Renderer* renderer) const override;

    // True while the gate is currently blocking cars from passing.
    bool isClosed() const { return m_closed; }
    void setClosed(bool closed) { m_closed = closed; }

    // Two gates placed on opposite sides of the circuit, each with independently
    // randomized open/close timing (see kMin/MaxClosedDuration, kMin/MaxOpenDuration).
    static std::vector<Gate> createInitialGates(const Track& track, float trackWidth);
    static std::vector<Gate> createInitialGates(const Track& track, float trackWidth,
                                                std::mt19937& rng);

private:
    bool m_closed = true;      // current open/closed state
    float m_phaseTimer = 0.0f; // seconds remaining until the next random toggle
    std::mt19937 m_rng;
};
