#pragma once

#include <SDL2/SDL.h>
#include <vector>

#include "Actor.h"

class Car;
class Track;

// A static obstacle sitting on the circuit. Its world position is computed once at
// construction from the track's centerline sample so rendering and collision checks
// stay cheap on every frame.
class Rock : public Actor {
public:
    float s;              // arc-length along the track centerline
    float laneOffset;     // perpendicular offset from the centerline
    float size = 20.0f;   // approximate half-extent used for drawing and AABB collision
    // World-space center is stored in the inherited Actor::position, precomputed at
    // construction from the track sample so rendering/collision don't retrig each frame.

    Rock(float s, float laneOffset, const Track& track, float size = 20.0f);

    void render(SDL_Renderer* renderer) const override;

    // Builds a fixed set of rocks scattered along the circuit. Placement stays outside
    // the AI cars' fixed lanes (|laneOffset| == 25), so only players who steer wide
    // ever collide with a rock -- AI cars can't change lanes and would otherwise get
    // stuck against an in-lane rock forever.
    static std::vector<Rock> createInitialRocks(const Track& track);

    // Detects car-vs-rock overlap and, when found, zeroes the car's speed, arms its
    // recovery boost, and pushes the car back along the track (away from the rock).
    static void resolveCarCollisions(std::vector<Car>& cars,
                                       const std::vector<Rock>& rocks,
                                       float totalLength,
                                       float carLength, float carWidthDim);
};
