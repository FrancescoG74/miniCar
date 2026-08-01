#pragma once

#include <SDL3/SDL.h>
#include <array>

class RaceSession;
class Track;
class StartLine;

// Draws the simulated world -- track surface, start line, rocks, gates and
// cars -- each frame. Pure presentation: reads Track/StartLine/RaceSession
// and owns no state itself, so it stays trivially reusable if a second
// rendering surface (e.g. a minimap) is ever added.
class WorldRenderer {
public:
    // Sprite the caller has already created/tinted; WorldRenderer only draws it.
    struct CarSprite {
        SDL_Texture* texture = nullptr;
        int width = 0;
        int height = 0;
    };

    // Textures indexed by Rock::variant; a null entry falls back to Rock's
    // procedural polygon draw.
    using RockSprites = std::array<SDL_Texture*, 5>;

    static void render(SDL_Renderer* renderer, const Track& track, const StartLine& startLine,
                        const RaceSession& race, const CarSprite& carSprite,
                        const RockSprites& rockSprites);
};
