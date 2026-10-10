#pragma once

#include <SDL3/SDL.h>
#include <array>
#include <atomic>

#include "game/RaceConstants.h"

// Synthesizes a simple looping engine drone for up to 6 cars using raw SDL audio
// (no extra dependencies like SDL3_mixer). Each car's pitch/volume is driven by
// a normalized speed value that can be updated from the main thread at any time.
class EngineSound {
public:
    static constexpr int kMaxCars = static_cast<int>(kInitialCarCount);

    EngineSound();
    ~EngineSound();

    // Opens the audio device and starts playback. Returns false (and logs a
    // warning) if audio is unavailable; the caller can continue without sound.
    bool init();
    void shutdown();

    // normalizedSpeed should be in [0, 1]; values outside are clamped. Only
    // records the target pitch/volume for this car -- does not touch the
    // audio stream itself, so it's cheap to call as often as needed.
    void setCarSpeed(int carIndex, float normalizedSpeed);

    // Synthesizes `dt` seconds of audio mixing every car and queues it once.
    // Call exactly once per frame (regardless of how many times
    // setCarSpeed() was called that frame) so the stream isn't over-fed.
    void update(float dt);

private:
    void generate(Sint16* buffer, int numSamples);

    SDL_AudioDeviceID m_device = 0;
    SDL_AudioStream* m_stream = nullptr;
    int m_sampleRate = 44100;

    std::array<std::atomic<float>, kMaxCars> m_targetSpeed{};
    // Smoothed per-car speed and oscillator phases. Only ever touched from
    // the main thread (via update()/generate()), same as m_targetSpeed's
    // writer side, so plain (non-atomic) state is fine here.
    std::array<float, kMaxCars> m_smoothedSpeed{};
    std::array<double, kMaxCars> m_phase{};
    std::array<double, kMaxCars> m_wobblePhase{};

    // Fractional leftover samples carried across frames so dt*sampleRate
    // rounding never drifts the audio clock.
    double m_sampleAccumulator = 0.0;
};

// Plays short one-shot beep tones for UI cues (e.g. the pre-race countdown).
// Uses its own SDL audio device driven via SDL_QueueAudio rather than a
// callback, since these are one-shot blips rather than a continuous synth.
class UiSound {
public:
    UiSound() = default;
    ~UiSound();

    // Opens the audio device. Returns false (and logs a warning) if audio is
    // unavailable; the caller can continue without sound.
    bool init();
    void shutdown();
    bool isAvailable() const { return m_device != 0; }

    // Queues a short sine-wave beep at the given frequency/duration. Safe to
    // call even if init() failed or was never called (silent no-op).
    void playBeep(float frequencyHz, float durationSeconds, float volume = 0.5f);

private:
    SDL_AudioDeviceID m_device = 0;
    SDL_AudioStream* m_stream = nullptr;
    int m_sampleRate = 44100;
};
