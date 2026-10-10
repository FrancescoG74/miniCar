#include "Audio.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

EngineSound::EngineSound() {
    for (auto& s : m_targetSpeed) s.store(0.0f, std::memory_order_relaxed);
    m_smoothedSpeed.fill(0.0f);
    m_phase.fill(0.0);
    m_wobblePhase.fill(0.0);
}

EngineSound::~EngineSound() {
    shutdown();
}

bool EngineSound::init() {
    SDL_AudioSpec desired{};
    desired.freq = m_sampleRate;
    desired.format = SDL_AUDIO_S16;
    desired.channels = 1;

    m_stream = SDL_CreateAudioStream(&desired, &desired);
    if (!m_stream) {
        std::cerr << "SDL_CreateAudioStream failed (continuing without sound): "
                   << SDL_GetError() << std::endl;
        return false;
    }

    m_device = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &desired);
    if (m_device == 0) {
        std::cerr << "SDL_OpenAudioDevice failed (continuing without sound): "
                   << SDL_GetError() << std::endl;
        SDL_DestroyAudioStream(m_stream);
        m_stream = nullptr;
        return false;
    }

    if (!SDL_BindAudioStream(m_device, m_stream)) {
        std::cerr << "SDL_BindAudioStream failed (continuing without sound): "
                   << SDL_GetError() << std::endl;
        SDL_DestroyAudioStream(m_stream);
        m_stream = nullptr;
        SDL_CloseAudioDevice(m_device);
        m_device = 0;
        return false;
    }

    // Pre-fill stream with initial silence
    const int bufferSize = m_sampleRate / 10;  // 100ms buffer
    std::vector<Sint16> initialBuffer(bufferSize, 0);
    SDL_PutAudioStreamData(m_stream, initialBuffer.data(), bufferSize * sizeof(Sint16));

    SDL_ResumeAudioDevice(m_device); // start playback
    return true;
}

void EngineSound::shutdown() {
    if (m_device != 0) {
        SDL_CloseAudioDevice(m_device);
        m_device = 0;
    }
    if (m_stream) {
        SDL_DestroyAudioStream(m_stream);
        m_stream = nullptr;
    }
}

void EngineSound::setCarSpeed(int carIndex, float normalizedSpeed) {
    if (carIndex < 0 || carIndex >= kMaxCars) return;
    float clamped = std::clamp(normalizedSpeed, 0.0f, 1.0f);
    m_targetSpeed[carIndex].store(clamped, std::memory_order_relaxed);
}

void EngineSound::update(float dt) {
    if (!m_stream || m_device == 0 || dt <= 0.0f) return;

    // Guard against a huge dt (debugger pause, app backgrounded) turning
    // into a giant synthesis burst.
    constexpr float kMaxDt = 0.1f;
    dt = std::min(dt, kMaxDt);

    // If we're already comfortably ahead, skip this frame instead of piling
    // more audio on top -- this is what previously caused the engine sound
    // to sound laggy/unrealistic: setCarSpeed() used to queue a fresh 50ms
    // chunk on *every* call, and it was called once per car per frame, so
    // several hundred ms of audio could be queued every single frame.
    constexpr int kMaxQueuedBytes = 44100 * static_cast<int>(sizeof(Sint16)) / 5; // ~200ms
    if (SDL_GetAudioStreamQueued(m_stream) > kMaxQueuedBytes) return;

    m_sampleAccumulator += static_cast<double>(dt) * m_sampleRate;
    int numSamples = static_cast<int>(m_sampleAccumulator);
    m_sampleAccumulator -= numSamples;
    if (numSamples <= 0) return;

    std::vector<Sint16> buffer(numSamples);
    generate(buffer.data(), numSamples);
    SDL_PutAudioStreamData(m_stream, buffer.data(), numSamples * static_cast<int>(sizeof(Sint16)));
}

void EngineSound::generate(Sint16* buffer, int numSamples) {
    constexpr double kTwoPi = 6.283185307179586;
    // How fast the synthesized pitch/volume may chase the real target speed
    // per second. Without this, a sudden setCarSpeed() jump (e.g. after a
    // collision zeroes the car's speed) snaps the pitch instantly, which
    // reads as a glitch rather than an engine revving down.
    constexpr float kSlewPerSecond = 4.0f;
    const float maxStep = kSlewPerSecond / static_cast<float>(m_sampleRate);

    for (int i = 0; i < numSamples; ++i) {
        double mixed = 0.0;

        for (int c = 0; c < kMaxCars; ++c) {
            const float target = m_targetSpeed[c].load(std::memory_order_relaxed);
            float& smooth = m_smoothedSpeed[c];
            smooth += std::clamp(target - smooth, -maxStep, maxStep);
            const float speed = smooth;

            // Idle firing frequency rising toward redline, roughly matching
            // a small engine's audible range (~45Hz idle to ~230Hz redline).
            const double freq = 45.0 + static_cast<double>(speed) * 185.0;
            m_phase[c] += freq / m_sampleRate;
            if (m_phase[c] >= 1.0) m_phase[c] -= 1.0;

            // Slow amplitude wobble that fades out as revs climb, giving the
            // idle an uneven "putt-putt" character instead of a pure tone.
            m_wobblePhase[c] += 9.0 / m_sampleRate;
            if (m_wobblePhase[c] >= 1.0) m_wobblePhase[c] -= 1.0;
            const double wobble = 1.0 + 0.18 * (1.0 - speed) * std::sin(kTwoPi * m_wobblePhase[c]);

            // Blend a clean sine (smooth idle) into an increasingly harsh
            // sawtooth (aggressive high-rev growl), plus a slightly detuned
            // 2nd harmonic and a quiet 3rd for a richer, less synthetic timbre.
            const double sine = std::sin(kTwoPi * m_phase[c]);
            const double saw = 2.0 * (m_phase[c] - std::floor(m_phase[c] + 0.5));
            const double fundamental = sine * (1.0 - 0.6 * speed) + saw * (0.3 + 0.6 * speed);
            const double harmonic2 = std::sin(kTwoPi * m_phase[c] * 2.003) * 0.35 * speed;
            const double harmonic3 = std::sin(kTwoPi * m_phase[c] * 3.0) * 0.15;

            const double amp = (0.05 + 0.12 * speed) * wobble;
            mixed += (fundamental + harmonic2 + harmonic3) * amp;
        }

        mixed /= kMaxCars;
        // Soft-clip instead of hard clamp so overlapping engines compress
        // smoothly rather than crackle at the ceiling.
        mixed = std::tanh(mixed * 1.6);
        buffer[i] = static_cast<Sint16>(std::clamp(mixed, -1.0, 1.0) * 32767.0);
    }
}

UiSound::~UiSound() {
    shutdown();
}

bool UiSound::init() {
    SDL_AudioSpec desired{};
    desired.freq = m_sampleRate;
    desired.format = SDL_AUDIO_S16;
    desired.channels = 1;

    m_stream = SDL_CreateAudioStream(&desired, &desired);
    if (!m_stream) {
        std::cerr << "SDL_CreateAudioStream (UI sounds) failed (continuing without sound): "
                   << SDL_GetError() << std::endl;
        return false;
    }

    m_device = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &desired);
    if (m_device == 0) {
        std::cerr << "SDL_OpenAudioDevice (UI sounds) failed (continuing without sound): "
                   << SDL_GetError() << std::endl;
        SDL_DestroyAudioStream(m_stream);
        m_stream = nullptr;
        return false;
    }

    if (!SDL_BindAudioStream(m_device, m_stream)) {
        std::cerr << "SDL_BindAudioStream (UI sounds) failed (continuing without sound): "
                   << SDL_GetError() << std::endl;
        SDL_DestroyAudioStream(m_stream);
        m_stream = nullptr;
        SDL_CloseAudioDevice(m_device);
        m_device = 0;
        return false;
    }

    SDL_ResumeAudioDevice(m_device); // start playback
    return true;
}

void UiSound::shutdown() {
    if (m_device != 0) {
        SDL_CloseAudioDevice(m_device);
        m_device = 0;
    }
    if (m_stream) {
        SDL_DestroyAudioStream(m_stream);
        m_stream = nullptr;
    }
}

void UiSound::playBeep(float frequencyHz, float durationSeconds, float volume) {
    if (m_device == 0 || !m_stream) return;

    constexpr double kPi = 3.14159265358979323846;
    constexpr double kFadeSeconds = 0.01; // short fade in/out to avoid clicks

    const int numSamples = static_cast<int>(static_cast<float>(m_sampleRate) * durationSeconds);
    std::vector<Sint16> buffer(numSamples);
    for (int i = 0; i < numSamples; ++i) {
        double t = static_cast<double>(i) / m_sampleRate;
        double envelope = std::min({ 1.0, t / kFadeSeconds, (durationSeconds - t) / kFadeSeconds });
        envelope = std::clamp(envelope, 0.0, 1.0);
        double sample = std::sin(2.0 * kPi * frequencyHz * t) * volume * envelope;
        buffer[i] = static_cast<Sint16>(std::clamp(sample, -1.0, 1.0) * 32767.0);
    }

    SDL_PutAudioStreamData(m_stream, buffer.data(), static_cast<int>(buffer.size() * sizeof(Sint16)));
}
