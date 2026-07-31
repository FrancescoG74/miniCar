#pragma once

#include <SDL2/SDL.h>

#include "SdlRaii.h"
#include "game/GameState.h"

// Pre-race setup screen: lets the player pick how many humans are driving
// (0-2), their names, and the laps needed to win. Shown at startup and again
// every time 'R' is pressed. Owns keyboard input (typing into name fields)
// and replaces the world/HUD render with its own full-screen layout.
class MenuState : public GameState {
public:
    void onEnter(Game& game) override;
    void onExit(Game& game) override;
    void handleEvent(Game& game, const SDL_Event& event) override;
    void update(Game& game, float dt) override;
    void renderOverlay(Game& game) override;
    bool rendersWorld() const override { return false; }
    bool ownsInput() const override { return true; }

private:
    enum class Field { Players = 0, Player1Name = 1, Player2Name = 2, Laps = 3 };
    static constexpr int kFieldCount = 4;
    static constexpr int kMinLaps = 1;
    static constexpr int kMaxLaps = 20;
    static constexpr std::size_t kMaxNameLength = 14;

    void moveFocus(Game& game, int direction);
    void adjustValue(Game& game, int direction);
    void appendText(Game& game, const char* utf8Text);
    void backspace(Game& game);
    void rebuildTextures(Game& game);

    Field m_focus = Field::Players;
    bool m_dirty = true;
    // True when the next digit typed into the Laps field should replace the
    // value rather than append to it (reset whenever focus lands on Laps).
    bool m_lapsFresh = true;

    struct Line {
        SDL_TexturePtr texture;
        SDL_Rect rect{ 0, 0, 0, 0 };
    };
    SDL_TexturePtr m_titleTexture;
    SDL_Rect m_titleRect{ 0, 0, 0, 0 };
    SDL_TexturePtr m_instructionsTexture;
    SDL_Rect m_instructionsRect{ 0, 0, 0, 0 };
    Line m_lines[kFieldCount];
};

// The pre-race countdown. Freezes the world, ticks a digit from 3 down to "Go",
// cues audio (voice if available, beep otherwise), and transitions to the
// RacingState the moment the timer hits zero.
class CountdownState : public GameState {
public:
    void onEnter(Game& game) override;
    void update(Game& game, float dt) override;
    void renderOverlay(Game& game) override;
};

// The race is running: cars/gates tick, collisions resolve, engines whine,
// and the moment any car crosses the winning lap the state transitions to
// FinishedState.
class RacingState : public GameState {
public:
    void update(Game& game, float dt) override;
};

// Race is over: engines silenced, a semi-transparent overlay + winner banner
// on top of the still-rendered world. Press 'R' to return to the setup menu.
class FinishedState : public GameState {
public:
    void onEnter(Game& game) override;
    void update(Game& game, float dt) override;
    void renderOverlay(Game& game) override;
};
