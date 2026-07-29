#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

struct AppWindow {
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    TTF_Font* font = nullptr; // may be null if SDL_ttf init fails
    int width = 0;
    int height = 0;
};

// Initializes SDL (video + audio), SDL_ttf, the window and the renderer, and opens the
// default HUD font. Returns true on success; on failure logs to stderr, cleans up any
// partial state and leaves `app` zero-initialized.
bool initApp(AppWindow& app, int width, int height, const char* title);

// Reverses initApp(): destroys the window, renderer, font and quits SDL subsystems.
void shutdownApp(AppWindow& app);
