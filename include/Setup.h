#pragma once

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>

// RAII bundle for the window, renderer, HUD font and the underlying SDL / SDL_ttf /
// SDL_image subsystems. The destructor cleans up whatever `initApp` managed to open,
// so callers just declare an `AppWindow` on the stack and let it go out of scope --
// no explicit `shutdownApp` needed.
struct AppWindow {
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    TTF_Font* font = nullptr; // may be null if SDL_ttf init fails
    int width = 0;
    int height = 0;
    bool sdlInitialized = false;
    bool ttfInitialized = false;

    AppWindow() = default;
    ~AppWindow();

    // Owns raw SDL handles; copying/moving would double-free them.
    AppWindow(const AppWindow&) = delete;
    AppWindow& operator=(const AppWindow&) = delete;
    AppWindow(AppWindow&&) = delete;
    AppWindow& operator=(AppWindow&&) = delete;
};

// Initializes SDL (video + audio), SDL_ttf, the window and the renderer, and opens the
// default HUD font. Returns true on success. On failure it logs to stderr and leaves
// `app` in a partially-initialized state -- the destructor will still clean up any
// resources that were successfully acquired.
bool initApp(AppWindow& app, int width, int height, const char* title);
