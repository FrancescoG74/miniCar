#include "game/HudRenderer.h"

#include <algorithm>

#include "actor/Car.h"

void HudRenderer::renderLabel(SDL_Renderer* renderer, SDL_Texture* texture, const SDL_FRect& rect) {
    if (!texture) return;
    SDL_FRect background{ rect.x - 8.0f, rect.y - 6.0f, rect.w + 16.0f, rect.h + 12.0f };
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
    SDL_RenderFillRect(renderer, &background);
    
    SDL_RenderTexture(renderer, texture, nullptr, &rect);
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

    float rowY = 20.0f;
    for (size_t idx : order) {
        SDL_Texture* tex = hud[idx].texture.get();
        if (!tex) continue;
        SDL_FRect& r = hud[idx].rect;
        r.x = static_cast<float>(windowWidth) - 20.0f - r.w;
        r.y = rowY;
        
        SDL_FRect background{ r.x - 8.0f, rowY - 4.0f, r.w + 16.0f, r.h + 8.0f };
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
        SDL_RenderFillRect(renderer, &background);
        
        SDL_RenderTexture(renderer, tex, nullptr, &r);
        rowY += r.h + 8.0f;
    }
}
