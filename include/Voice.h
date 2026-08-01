#pragma once

#include <SDL3/SDL.h>
#include <string>

// Optional spoken-word audio cue (e.g. "3", "2", "1", "Go" for the pre-race
// countdown), synthesized with espeak-ng when it was found at build time (see
// CMakeLists.txt / MINICAR_HAVE_ESPEAK). If espeak-ng wasn't available on this
// machine, init() simply returns false and speak() is a silent no-op --
// callers should fall back to another cue (e.g. UiSound beeps).
class Voice {
public:
    Voice() = default;
    ~Voice();

    // Returns true if espeak-ng initialized and a playback device opened.
    bool init();
    void shutdown();

    // Synthesizes `text` and queues it for playback through its own SDL audio
    // device. Blocks briefly while espeak-ng synthesizes -- intended for short
    // phrases only (single words/numbers), not long text. No-op if init()
    // wasn't called or failed.
    void speak(const std::string& text);

private:
    SDL_AudioDeviceID m_device = 0;
    SDL_AudioStream* m_stream = nullptr;
    int m_sampleRate = 22050;
    bool m_available = false;
};
