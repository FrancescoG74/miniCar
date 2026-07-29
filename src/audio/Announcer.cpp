#include "audio/Announcer.h"

#include <string>

#include "Audio.h"
#include "Voice.h"

std::unique_ptr<Announcer> Announcer::create(Voice* voice, UiSound* beep, bool voiceAvailable) {
    if (voiceAvailable && voice) return std::make_unique<VoiceAnnouncer>(voice);
    if (beep && beep->isAvailable()) return std::make_unique<BeepAnnouncer>(beep);
    return std::make_unique<SilentAnnouncer>();
}

void VoiceAnnouncer::countdown(int digit) {
    if (!m_voice) return;
    m_voice->speak(digit > 0 ? std::to_string(digit) : std::string("Go"));
}

void BeepAnnouncer::countdown(int digit) {
    if (!m_beep) return;
    if (digit > 0) {
        m_beep->playBeep(440.0f, 0.15f);
    } else {
        m_beep->playBeep(880.0f, 0.3f);
    }
}
