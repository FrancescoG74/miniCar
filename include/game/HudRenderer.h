#pragma once

#include <SDL2/SDL.h>
#include <vector>

#include "SdlRaii.h"

class Car;

// One rebuildable HUD row for a single car's lap-count label: its texture,
// the rect it's blitted into, and the lap count it was last rebuilt for.
// Replaces three parallel vectors (carLapTextures/carLapRects/carLastLaps)
// that used to be indexed in lockstep by car index -- a classic parallel-
// array smell that made P2 join/leave HUD invalidation error-prone.
struct CarHudRow {
    SDL_TexturePtr texture;
    SDL_Rect rect{ 0, 0, 0, 0 };
    int lastLaps = -1;
};

// Draws the always-on HUD chrome: player name tags (top-left) and the
// lap-count leaderboard (top-right). Countdown/winner overlays are
// phase-specific and stay on GameState::renderOverlay implementations.
class HudRenderer {
public:
    // Draws a single label texture with a translucent black backing rect.
    // No-op if `texture` is null (e.g. Player 2 hasn't joined yet).
    static void renderLabel(SDL_Renderer* renderer, SDL_Texture* texture, const SDL_Rect& rect);

    // Sorts by lap count (then distance traveled as a tie-breaker) so the
    // current leader is on top, lays out each row along the right edge, and
    // draws it. Mutates each row's rect.x/rect.y in place every frame.
    static void renderLeaderboard(SDL_Renderer* renderer, int windowWidth,
                                   const std::vector<Car>& cars,
                                   std::vector<CarHudRow>& hud);
};
