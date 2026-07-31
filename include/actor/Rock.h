#pragma once

#include <SDL2/SDL.h>
#include <random>
#include <vector>

#include "actor/Actor.h"

class Track;

// A static obstacle sitting on the circuit. Its world position is computed once at
// construction from the track's centerline sample so rendering and collision checks
// stay cheap on every frame.
class Rock : public Actor {
public:
    float s;              // arc-length along the track centerline
    float laneOffset;     // perpendicular offset from the centerline
    float size = 20.0f;   // approximate half-extent used for drawing and AABB collision
    int variant = 0;      // which of the sprite image variants to draw (see Game::rockTextures)
    // World-space center is stored in the inherited Actor::position, precomputed at
    // construction from the track sample so rendering/collision don't retrig each frame.

    Rock(float s, float laneOffset, const Track& track, float size = 20.0f, int variant = 0);

    void render(SDL_Renderer* renderer) const override;

    // Draws using `texture` scaled/centered to fit the rock's size, preserving the
    // image's aspect ratio; falls back to the procedural polygon if texture is null.
    void render(SDL_Renderer* renderer, SDL_Texture* texture) const;

    // Builds a fixed set of rocks scattered along the circuit. Placement stays outside
    // the AI cars' fixed lanes (|laneOffset| == 25), so only players who steer wide
    // ever collide with a rock -- AI cars can't change lanes and would otherwise get
    // stuck against an in-lane rock forever.
    static std::vector<Rock> createInitialRocks(const Track& track);
    static std::vector<Rock> createInitialRocks(const Track& track, std::mt19937& rng);
};
