#include "Setup.h"

#include <iostream>

AppWindow::~AppWindow() {
    if (font) TTF_CloseFont(font);
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    if (imgInitialized) IMG_Quit();
    if (ttfInitialized) TTF_Quit();
    if (sdlInitialized) SDL_Quit();
}

bool initApp(AppWindow& app, int width, int height, const char* title) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return false;
    }
    app.sdlInitialized = true;

    if (TTF_Init() != 0) {
        std::cerr << "TTF_Init failed (continuing without on-screen labels): "
                   << TTF_GetError() << std::endl;
    } else {
        app.ttfInitialized = true;
    }

    if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == 0) {
        std::cerr << "IMG_Init failed (continuing without sprite textures): "
                   << IMG_GetError() << std::endl;
    } else {
        app.imgInitialized = true;
    }

    app.window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width, height,
        SDL_WINDOW_SHOWN
    );
    if (!app.window) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
        return false;
    }

    app.renderer = SDL_CreateRenderer(app.window, -1, SDL_RENDERER_ACCELERATED);
    if (!app.renderer) {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << std::endl;
        return false;
    }

    if (app.ttfInitialized) {
        app.font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 28);
        if (!app.font) {
            std::cerr << "TTF_OpenFont failed (continuing without on-screen labels): "
                       << TTF_GetError() << std::endl;
        }
    }

    app.width = width;
    app.height = height;
    return true;
}
