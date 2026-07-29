#pragma once

#include <SDL2/SDL.h>
#include <array>
#include <atomic>

// Synthesizes a simple looping engine drone for up to 6 cars using raw SDL audio
// (no extra dependencies like SDL2_mixer). Each car's pitch/volume is driven by
// a normalized speed value that can be updated from the main thread at any time.
class EngineSound {
public:
    static constexpr int kMaxCars = 6;

    EngineSound();
    ~EngineSound();

    // Opens the audio device and starts playback. Returns false (and logs a
    // warning) if audio is unavailable; the caller can continue without sound.
    bool init();
    void shutdown();

    // normalizedSpeed should be in [0, 1]; values outside are clamped.
    void setCarSpeed(int carIndex, float normalizedSpeed);

private:
    static void audioCallback(void* userdata, Uint8* stream, int len);
    void generate(Sint16* buffer, int numSamples);

    SDL_AudioDeviceID m_device = 0;
    int m_sampleRate = 44100;

    std::array<std::atomic<float>, kMaxCars> m_speed{};
    std::array<double, kMaxCars> m_phase{};
};
