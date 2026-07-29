#pragma once

#include <memory>

class Voice;
class UiSound;

// Strategy interface for the pre-race countdown audio cue. Pulls the
// `voiceAvailable ? voice.speak(...) : uiSound.playBeep(...)` decision out of
// CountdownState so that state code only needs `announcer->countdown(digit)`.
//
// The concrete strategy is picked once at startup based on which audio backends
// initialized successfully:
//   - Voice ready         -> VoiceAnnouncer (spoken "3", "2", "1", "Go")
//   - only UiSound ready  -> BeepAnnouncer  (short beep for 3/2/1, longer for go)
//   - neither ready       -> SilentAnnouncer (Null Object pattern)
class Announcer {
public:
    virtual ~Announcer() = default;

    // Called by CountdownState once per displayed digit. `digit` is 3..0
    // (0 == "Go" moment).
    virtual void countdown(int digit) = 0;

    // Factory: picks the best available backend given what initialized.
    // Both pointers are non-owning; the Game owns Voice and UiSound.
    static std::unique_ptr<Announcer> create(Voice* voice, UiSound* beep, bool voiceAvailable);
};

// Speaks the digit ("3"/"2"/"1"/"Go") through espeak-ng via Voice.
class VoiceAnnouncer : public Announcer {
public:
    explicit VoiceAnnouncer(Voice* voice) : m_voice(voice) {}
    void countdown(int digit) override;
private:
    Voice* m_voice;
};

// Falls back to short sine beeps: 440 Hz for 3/2/1, higher/longer 880 Hz for 0.
class BeepAnnouncer : public Announcer {
public:
    explicit BeepAnnouncer(UiSound* beep) : m_beep(beep) {}
    void countdown(int digit) override;
private:
    UiSound* m_beep;
};

// Null Object: no audio backend available -- countdown() is a silent no-op.
class SilentAnnouncer : public Announcer {
public:
    void countdown(int /*digit*/) override {}
};
