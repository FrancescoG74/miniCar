#include "Setup.h"

#include <iostream>

bool initApp(AppWindow& app, int width, int height, const char* title) {
    app = AppWindow{};

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return false;
    }

    if (TTF_Init() != 0) {
        std::cerr << "TTF_Init failed (continuing without on-screen labels): "
                   << TTF_GetError() << std::endl;
    }

    app.window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width, height,
        SDL_WINDOW_SHOWN
    );
    if (!app.window) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    app.renderer = SDL_CreateRenderer(app.window, -1, SDL_RENDERER_ACCELERATED);
    if (!app.renderer) {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(app.window);
        app.window = nullptr;
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    app.font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 28);
    if (!app.font) {
        std::cerr << "TTF_OpenFont failed (continuing without on-screen labels): "
                   << TTF_GetError() << std::endl;
    }

    app.width = width;
    app.height = height;
    return true;
}

void shutdownApp(AppWindow& app) {
    if (app.font) TTF_CloseFont(app.font);
    if (app.renderer) SDL_DestroyRenderer(app.renderer);
    if (app.window) SDL_DestroyWindow(app.window);
    TTF_Quit();
    SDL_Quit();
    app = AppWindow{};
}
