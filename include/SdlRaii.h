#pragma once

#include <SDL3/SDL.h>

#include <memory>

// Small RAII helpers around raw SDL handles so callers can rely on destructors
// instead of remembering matching SDL_DestroyXxx() calls. Only what miniCar
// currently uses is wrapped -- extend as needed.

struct SDL_TextureDeleter {
    void operator()(SDL_Texture* t) const noexcept {
        if (t) SDL_DestroyTexture(t);
    }
};

using SDL_TexturePtr = std::unique_ptr<SDL_Texture, SDL_TextureDeleter>;
