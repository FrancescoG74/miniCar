#include "Setup.h"

#include <iostream>
#include <string>

AppWindow::~AppWindow() {
    if (font) TTF_CloseFont(font);
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    if (ttfInitialized) TTF_Quit();
    if (sdlInitialized) SDL_Quit();
}

namespace {

// Loads the bundled HUD font (assets/DejaVuSans-Bold.ttf). On Android the
// font is packaged as an APK asset instead of a plain file on disk, so it
// must be opened through SDL's asset-manager-backed SDL_IOStream (a relative
// path handed to SDL_IOFromFile is read from the APK on that platform);
// everywhere else it's just a file next to the other bundled assets
// (rocks, etc.) referenced via MINICAR_ASSETS_DIR.
TTF_Font* openHudFont(float pointSize) {
#ifdef ANDROID
    SDL_IOStream* io = SDL_IOFromFile("DejaVuSans-Bold.ttf", "rb");
    if (!io) return nullptr;
    return TTF_OpenFontIO(io, true, pointSize);
#else
    const std::string path = std::string(MINICAR_ASSETS_DIR) + "/DejaVuSans-Bold.ttf";
    return TTF_OpenFont(path.c_str(), pointSize);
#endif
}

} // namespace

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

    // Renders as if the output were always `width`x`height`, then lets SDL
    // scale/letterbox that onto whatever the real window/display resolution
    // turns out to be. This keeps the (hardcoded, pixel-based) track and HUD
    // layout centered and screen-filling on any device -- critical on
    // Android, where the requested window size is not necessarily the actual
    // screen resolution.
    SDL_SetRenderLogicalPresentation(app.renderer, width, height, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    if (app.ttfInitialized) {
        app.font = openHudFont(28);
        if (!app.font) {
            std::cerr << "Failed to load bundled HUD font (continuing without on-screen labels): "
                       << SDL_GetError() << std::endl;
        }
    }

    app.width = width;
    app.height = height;
    return true;
}
