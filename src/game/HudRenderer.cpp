#include "game/HudRenderer.h"

#include <algorithm>

#include "actor/Car.h"

void HudRenderer::renderLabel(SDL_Renderer* renderer, SDL_Texture* texture, const SDL_Rect& rect) {
    if (!texture) return;
    SDL_Rect background{ rect.x - 8, rect.y - 6, rect.w + 16, rect.h + 12 };
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
    SDL_RenderFillRect(renderer, &background);
    SDL_RenderCopy(renderer, texture, nullptr, &rect);
}

void HudRenderer::renderLeaderboard(SDL_Renderer* renderer, int windowWidth,
                                    const std::vector<Car>& cars,
                                    std::vector<CarHudRow>& hud) {
    // Sort by lap count (then distance) so the current leader appears on top.
    std::vector<size_t> order(cars.size());
    for (size_t i = 0; i < order.size(); ++i) order[i] = i;
    std::sort(order.begin(), order.end(), [&](size_t a, size_t b) {
        if (cars[a].laps != cars[b].laps) return cars[a].laps > cars[b].laps;
        return cars[a].distanceTraveled > cars[b].distanceTraveled;
    });

    int rowY = 20;
    for (size_t idx : order) {
        SDL_Texture* tex = hud[idx].texture.get();
        if (!tex) continue;
        SDL_Rect& r = hud[idx].rect;
        r.x = windowWidth - 20 - r.w;
        r.y = rowY;
        SDL_Rect background{ r.x - 8, r.y - 4, r.w + 16, r.h + 8 };
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
        SDL_RenderFillRect(renderer, &background);
        SDL_RenderCopy(renderer, tex, nullptr, &r);
        rowY += r.h + 8;
    }
}
