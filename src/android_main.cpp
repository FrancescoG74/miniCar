#ifdef ANDROID

#include <android/log.h>
#include <SDL3/SDL.h>
#include "game/Game.h"

// Android log tag
#define LOG_TAG "miniCar"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

extern "C" int SDL_main(int /*argc*/, char* /*argv*/[]) {
    LOGI("Starting miniCar on Android");

    try {
        Game game;

        // The world/HUD layout uses hardcoded pixel constants authored for a
        // 1600x900 canvas (see Game::kTrackCenterX/Y). Requesting that same
        // logical size here -- matching desktop's main.cpp -- keeps the
        // circuit centered; Setup.cpp's SDL_SetRenderLogicalPresentation then
        // letterbox-scales it onto the device's actual screen resolution,
        // whatever that turns out to be.
        int windowWidth = 1600;
        int windowHeight = 900;

        if (!game.init(windowWidth, windowHeight, "miniCar")) {
            LOGE("Failed to initialize game");
            return 1;
        }

        LOGI("Game initialized successfully, starting main loop");
        int result = game.run();
        LOGI("Game loop ended with code: %d", result);
        return result;
    } catch (const std::exception& e) {
        LOGE("Exception in game main: %s", e.what());
        return 1;
    } catch (...) {
        LOGE("Unknown exception in game main");
        return 1;
    }
}

#endif // ANDROID

