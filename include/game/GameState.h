#pragma once

#include <SDL2/SDL.h>

class Game;

// State pattern base for the three race phases (Countdown, Racing, Finished).
// The Game owns all shared data; each state just controls what happens per
// frame in its phase and draws its phase-specific HUD overlay.
class GameState {
public:
    virtual ~GameState() = default;

    // Called by Game when this state becomes / stops being the active one.
    virtual void onEnter(Game& /*game*/) {}
    virtual void onExit(Game& /*game*/) {}

    // Called for every SDL event the Game hasn't already consumed
    // (Esc / R / P1-P2 toggles are handled centrally by Game).
    virtual void handleEvent(Game& /*game*/, const SDL_Event& /*event*/) {}

    // Advances one frame of state-specific logic (physics/AI/audio cues).
    virtual void update(Game& game, float dt) = 0;

    // Draws state-specific overlays on top of the shared world+HUD render.
    virtual void renderOverlay(Game& /*game*/) {}

    // False for full-screen states (e.g. the setup menu) that replace the
    // world/HUD render entirely instead of overlaying it.
    virtual bool rendersWorld() const { return true; }

    // True while this state wants to own all keyboard input (e.g. typing into
    // text fields), suppressing Game's universal Esc/R/1/2 shortcuts.
    virtual bool ownsInput() const { return false; }
};
