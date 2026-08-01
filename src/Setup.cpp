#include "Setup.h"

#include <iostream>

AppWindow::~AppWindow() {
    if (font) TTF_CloseFont(font);
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    if (ttfInitialized) TTF_Quit();
    if (sdlInitialized) SDL_Quit();
}

bool initApp(AppWindow& app, int width, int height, const char* title) {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return false;
    }
    app.sdlInitialized = true;

    if (!TTF_Init()) {
        std::cerr << "TTF_Init failed (continuing without on-screen labels): "
                   << SDL_GetError() << std::endl;
    } else {
        app.ttfInitialized = true;
    }

    // SDL3_image initializes automatically when needed, no IMG_Init required

    app.window = SDL_CreateWindow(
        title,
        width, height,
        0  // SDL3 windows are shown by default; use 0 for default flags
    );
    if (!app.window) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
        return false;
    }

    SDL_SetWindowPosition(app.window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

    app.renderer = SDL_CreateRenderer(app.window, nullptr);
    if (!app.renderer) {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << std::endl;
        return false;
    }

    if (app.ttfInitialized) {
        app.font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 28);
        if (!app.font) {
            std::cerr << "TTF_OpenFont failed (continuing without on-screen labels): "
                       << SDL_GetError() << std::endl;
        }
    }

    app.width = width;
    app.height = height;
    return true;
}
