#include "Audio.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

EngineSound::EngineSound() {
    for (auto& s : m_speed) s.store(0.0f, std::memory_order_relaxed);
    m_phase.fill(0.0);
}

EngineSound::~EngineSound() {
    shutdown();
}

bool EngineSound::init() {
    SDL_AudioSpec desired{};
    desired.freq = m_sampleRate;
    desired.format = AUDIO_S16SYS;
    desired.channels = 1;
    desired.samples = 1024;
    desired.callback = &EngineSound::audioCallback;
    desired.userdata = this;

    SDL_AudioSpec obtained{};
    m_device = SDL_OpenAudioDevice(nullptr, 0, &desired, &obtained, 0);
    if (m_device == 0) {
        std::cerr << "SDL_OpenAudioDevice failed (continuing without sound): "
                   << SDL_GetError() << std::endl;
        return false;
    }

    m_sampleRate = obtained.freq;
    SDL_PauseAudioDevice(m_device, 0); // start playback
    return true;
}

void EngineSound::shutdown() {
    if (m_device != 0) {
        SDL_CloseAudioDevice(m_device);
        m_device = 0;
    }
}

void EngineSound::setCarSpeed(int carIndex, float normalizedSpeed) {
    if (carIndex < 0 || carIndex >= kMaxCars) return;
    float clamped = std::clamp(normalizedSpeed, 0.0f, 1.0f);
    m_speed[carIndex].store(clamped, std::memory_order_relaxed);
}

void EngineSound::audioCallback(void* userdata, Uint8* stream, int len) {
    auto* self = static_cast<EngineSound*>(userdata);
    self->generate(reinterpret_cast<Sint16*>(stream), len / static_cast<int>(sizeof(Sint16)));
}

void EngineSound::generate(Sint16* buffer, int numSamples) {
    for (int i = 0; i < numSamples; ++i) {
        double sample = 0.0;

        for (int c = 0; c < kMaxCars; ++c) {
            float speed = m_speed[c].load(std::memory_order_relaxed);

            // Idle-to-redline pitch sweep, plus a touch of extra harmonic buzz at high speed.
            double freq = 55.0 + static_cast<double>(speed) * 130.0;
            m_phase[c] += freq / m_sampleRate;
            if (m_phase[c] >= 1.0) m_phase[c] -= 1.0;

            double saw = 2.0 * (m_phase[c] - std::floor(m_phase[c] + 0.5)); // range [-1, 1)
            double amp = 0.08 + 0.10 * speed;
            sample += saw * amp;
        }

        sample /= kMaxCars;
        sample = std::clamp(sample, -1.0, 1.0);
        buffer[i] = static_cast<Sint16>(sample * 32767.0);
    }
}

UiSound::~UiSound() {
    shutdown();
}

bool UiSound::init() {
    SDL_AudioSpec desired{};
    desired.freq = m_sampleRate;
    desired.format = AUDIO_S16SYS;
    desired.channels = 1;
    desired.samples = 1024;
    desired.callback = nullptr; // one-shot sounds are pushed via SDL_QueueAudio

    SDL_AudioSpec obtained{};
    m_device = SDL_OpenAudioDevice(nullptr, 0, &desired, &obtained, 0);
    if (m_device == 0) {
        std::cerr << "SDL_OpenAudioDevice (UI sounds) failed (continuing without sound): "
                   << SDL_GetError() << std::endl;
        return false;
    }

    m_sampleRate = obtained.freq;
    SDL_PauseAudioDevice(m_device, 0); // start playback
    return true;
}

void UiSound::shutdown() {
    if (m_device != 0) {
        SDL_CloseAudioDevice(m_device);
        m_device = 0;
    }
}

void UiSound::playBeep(float frequencyHz, float durationSeconds, float volume) {
    if (m_device == 0) return;

    constexpr double kPi = 3.14159265358979323846;
    constexpr double kFadeSeconds = 0.01; // short fade in/out to avoid clicks

    const int numSamples = static_cast<int>(m_sampleRate * durationSeconds);
    std::vector<Sint16> buffer(numSamples);
    for (int i = 0; i < numSamples; ++i) {
        double t = static_cast<double>(i) / m_sampleRate;
        double envelope = std::min({ 1.0, t / kFadeSeconds, (durationSeconds - t) / kFadeSeconds });
        envelope = std::clamp(envelope, 0.0, 1.0);
        double sample = std::sin(2.0 * kPi * frequencyHz * t) * volume * envelope;
        buffer[i] = static_cast<Sint16>(std::clamp(sample, -1.0, 1.0) * 32767.0);
    }

    SDL_QueueAudio(m_device, buffer.data(), static_cast<Uint32>(buffer.size() * sizeof(Sint16)));
}
