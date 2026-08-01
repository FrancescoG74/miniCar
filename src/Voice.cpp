#include "Voice.h"

#include <iostream>
#include <vector>

#ifdef MINICAR_HAVE_ESPEAK

// Minimal subset of the espeak-ng C API (speak_lib.h), declared by hand since
// only the runtime library (libespeak-ng.so.1) is available here, not the
// -dev package's headers. This mirrors the long-stable, unchanged espeak /
// espeak-ng public ABI.
extern "C" {

typedef enum {
    AUDIO_OUTPUT_PLAYBACK = 0,
    AUDIO_OUTPUT_RETRIEVAL = 1,
    AUDIO_OUTPUT_SYNCHRONOUS = 2,
    AUDIO_OUTPUT_SYNCH_PLAYBACK = 3
} espeak_AUDIO_OUTPUT;

typedef enum {
    EE_OK = 0,
    EE_INTERNAL_ERROR = -1,
    EE_BUFFER_FULL = 1,
    EE_NOT_FOUND = 2
} espeak_ERROR;

typedef enum {
    POS_CHARACTER = 1,
    POS_WORD = 2,
    POS_SENTENCE = 3
} espeak_POSITION_TYPE;

typedef enum {
    espeakEVENT_LIST_TERMINATED = 0,
    espeakEVENT_WORD = 1,
    espeakEVENT_SENTENCE = 2,
    espeakEVENT_MARK = 3,
    espeakEVENT_PLAY = 4,
    espeakEVENT_END = 5,
    espeakEVENT_MSG_TERMINATED = 6,
    espeakEVENT_PHONEME = 7,
    espeakEVENT_SAMPLERATE = 8
} espeak_EVENT_TYPE;

typedef struct {
    espeak_EVENT_TYPE type;
    int unique_identifier;
    int text_position;
    int length;
    int audio_position;
    int sample;
    void* user_data;
    union {
        int number;
        const char* name;
        char string[8];
    } id;
} espeak_EVENT;

typedef int (t_espeak_callback)(short*, int, espeak_EVENT*);

int espeak_Initialize(espeak_AUDIO_OUTPUT output, int buflength, const char* path, int options);
void espeak_SetSynthCallback(t_espeak_callback* callback);
espeak_ERROR espeak_SetVoiceByName(const char* name);
espeak_ERROR espeak_Synth(const void* text, size_t size, unsigned int position,
                           espeak_POSITION_TYPE position_type, unsigned int end_position,
                           unsigned int flags, unsigned int* unique_identifier, void* user_data);
espeak_ERROR espeak_Synchronize(void);
espeak_ERROR espeak_Terminate(void);

} // extern "C"

namespace {

// Accumulates PCM samples from the espeak synthesis callback for the single
// utterance currently in flight. Voice::speak() is synchronous (it calls
// espeak_Synchronize before returning), so one shared buffer is safe even
// though the callback is a plain C function pointer with no user-data capture.
std::vector<short>* g_captureBuffer = nullptr;

int synthCallback(short* wav, int numsamples, espeak_EVENT* /*events*/) {
    if (g_captureBuffer && wav && numsamples > 0) {
        g_captureBuffer->insert(g_captureBuffer->end(), wav, wav + numsamples);
    }
    return 0; // 0 = continue synthesis
}

} // namespace

#endif // MINICAR_HAVE_ESPEAK

Voice::~Voice() {
    shutdown();
}

bool Voice::init() {
#ifdef MINICAR_HAVE_ESPEAK
    // AUDIO_OUTPUT_RETRIEVAL: espeak-ng doesn't open its own audio device; it
    // hands us raw PCM via the callback and we play it through our own SDL
    // audio device instead, consistent with EngineSound/UiSound.
    int rate = espeak_Initialize(AUDIO_OUTPUT_RETRIEVAL, 0, nullptr, 0);
    if (rate <= 0) {
        std::cerr << "espeak_Initialize failed (continuing without voice)" << std::endl;
        return false;
    }
    m_sampleRate = rate;
    espeak_SetSynthCallback(&synthCallback);
    espeak_SetVoiceByName("en");

    SDL_AudioSpec desired{};
    desired.freq = m_sampleRate;
    desired.format = SDL_AUDIO_S16;
    desired.channels = 1;

    m_stream = SDL_CreateAudioStream(&desired, &desired);
    if (!m_stream) {
        std::cerr << "SDL_CreateAudioStream (voice) failed (continuing without voice): "
                   << SDL_GetError() << std::endl;
        return false;
    }

    m_device = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &desired);
    if (m_device == 0) {
        std::cerr << "SDL_OpenAudioDevice (voice) failed (continuing without voice): "
                   << SDL_GetError() << std::endl;
        SDL_DestroyAudioStream(m_stream);
        m_stream = nullptr;
        return false;
    }

    if (!SDL_BindAudioStream(m_device, m_stream)) {
        std::cerr << "SDL_BindAudioStream (voice) failed (continuing without voice): "
                   << SDL_GetError() << std::endl;
        SDL_DestroyAudioStream(m_stream);
        m_stream = nullptr;
        SDL_CloseAudioDevice(m_device);
        m_device = 0;
        return false;
    }

    SDL_ResumeAudioDevice(m_device);
    m_available = true;
    return true;
#else
    return false;
#endif
}

void Voice::shutdown() {
#ifdef MINICAR_HAVE_ESPEAK
    if (m_device != 0) {
        SDL_CloseAudioDevice(m_device);
        m_device = 0;
    }
    if (m_stream) {
        SDL_DestroyAudioStream(m_stream);
        m_stream = nullptr;
    }
    if (m_available) {
        espeak_Terminate();
        m_available = false;
    }
#endif
}

void Voice::speak(const std::string& text) {
#ifdef MINICAR_HAVE_ESPEAK
    if (!m_available || m_device == 0 || !m_stream) return;

    std::vector<short> samples;
    g_captureBuffer = &samples;
    unsigned int uid = 0;
    espeak_Synth(text.c_str(), text.size() + 1, 0, POS_CHARACTER, 0, 0, &uid, nullptr);
    espeak_Synchronize();
    g_captureBuffer = nullptr;

    if (!samples.empty()) {
        SDL_PutAudioStreamData(m_stream, samples.data(), static_cast<int>(samples.size() * sizeof(short)));
    }
#else
    (void)text;
#endif
}
