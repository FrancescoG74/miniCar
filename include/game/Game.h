#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <array>
#include <memory>
#include <string>
#include <vector>

#include "Audio.h"
#include "SdlRaii.h"
#include "Setup.h"
#include "Track.h"
#include "Voice.h"
#include "actor/Car.h"
#include "actor/StartLine.h"
#include "audio/Announcer.h"
#include "game/HudRenderer.h"
#include "game/RaceSession.h"
#include "game/TouchControls.h"

class GameState;

// Player/lap selections made in the setup menu, applied by Game::startRace().
// Names are optional; an empty name keeps the car's auto-assigned color name.
struct RaceSetupOptions {
    int playerCount = 1; // 0 = all AI, 1 = Player1 only, 2 = Player1 + Player2
    std::string player1Name;
    std::string player2Name;
    int laps = 5;
};

// Top-level owner of window/renderer/audio/world/HUD, plus the current
// GameState. `run()` drives the fixed init -> loop -> shutdown lifecycle.
//
// Public data is intentional: GameState subclasses read and mutate world/HUD
// fields directly. They're all trivially owned by the Game (RAII), so the
// destructor cleans everything up in the right order.
class Game {
public:
    // Tuning constants pulled out of the old main().
    static constexpr float kTrackCenterX = 800.0f;
    static constexpr float kTrackCenterY = 450.0f;
    static constexpr float kTrackStraight = 500.0f;
    static constexpr float kTrackRadius = 190.0f;
    static constexpr float kTrackWidth = 130.0f;
    static constexpr int kCarWidth = 16;
    static constexpr int kCarHeight = 30;

    static constexpr float kCountdownDuration = 4.0f;

    // -- Lifecycle ---------------------------------------------------------
    Game();
    ~Game();

    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;

    // Opens window/renderer/audio and builds the first race. Returns false on
    // fatal init failure; the destructor still cleans up whatever was opened.
    bool init(int windowWidth, int windowHeight, const char* title);

    // Blocking game loop. Returns the process exit code (0 on clean exit).
    int run();

    // -- State machine ----------------------------------------------------
    void transitionTo(std::unique_ptr<GameState> next);
    void quit() { m_running = false; }
    GameState* state() { return m_state.get(); }

    // Transitions to the setup menu without touching the current race world
    // (MenuState doesn't render it, so it can stay stale behind the menu).
    void showMenu();

    // Wipes and rebuilds the per-race world using `raceSetup` (cars, rocks,
    // gates, player assignment, HUD state) and reenters the Countdown state.
    void startRace();

    RaceSession& race() { return *m_race; }
    const RaceSession& race() const { return *m_race; }
    // Sets each car's target engine pitch/volume from its current speed and
    // synthesizes exactly `dt` seconds of audio. Call once per frame from
    // whichever GameState is active (racing or otherwise) so the engines
    // idle down smoothly instead of just cutting to silence.
    void synchronizeEngineSound(float dt);

    // -- Rendering helpers ------------------------------------------------
    // Rebuilds any car's lap texture whose lap count changed. Called every frame.
    void refreshLapTextures();
    // Rebuilds player1/2 label textures using the currently-driven cars' colors.
    void rebuildPlayerLabels();

    SDL_TexturePtr makeLabelTexture(const char* text, SDL_Color color, SDL_FRect& outRect);

    // -- Public shared data (owned by Game, accessed by states) -----------
    AppWindow app;
    int windowWidth = 0;
    int windowHeight = 0;

    std::unique_ptr<Track> track;
    std::unique_ptr<StartLine> startLine;
    SDL_TexturePtr carTexture;
    std::array<SDL_TexturePtr, 5> rockTextures; // indexed by Rock::variant

    // Player 1/2 labels in the top-left corner.
    SDL_TexturePtr player1LabelTexture;
    SDL_TexturePtr player2LabelTexture;
    SDL_FRect player1LabelRect{ 20, 20, 0, 0 };
    SDL_FRect player2LabelRect{ 20, 20, 0, 0 };

    // Per-car lap HUD stacked on the right, one row per car (indexed the same
    // as RaceSession::cars()). Drawn by HudRenderer::renderLeaderboard.
    std::vector<CarHudRow> carHud;

    // Countdown overlay.
    float countdownTimer = 0.0f;
    SDL_TexturePtr countdownTexture;
    SDL_FRect countdownRect{ 0, 0, 0, 0 };
    int countdownLastDigit = -2;

    // Race outcome presentation.
    SDL_TexturePtr winnerTexture;
    SDL_FRect winnerRect{ 0, 0, 0, 0 };
    SDL_TexturePtr winnerHintTexture;
    SDL_FRect winnerHintRect{ 0, 0, 0, 0 };

    // Audio backends.
    EngineSound engineSound;
    UiSound uiSound;
    Voice voice;
    bool voiceAvailable = false;
    std::unique_ptr<Announcer> announcer;

    // Menu selections; edited by MenuState, consumed by startRace().
    RaceSetupOptions raceSetup;

    // Virtual on-screen steering/pedals for touchscreens. Enabled when a
    // touch device is detected (always on Android); rendered/handled by
    // RacingState.
    TouchControls touchControls;

private:
    void pollEvents();
    void renderFrame();
    void applyPendingState();
    void updateWindowTitle();

    RaceTuning m_raceTuning;
    AiTuning m_aiTuning;
    std::unique_ptr<RaceSession> m_race;
    std::unique_ptr<GameState> m_state;
    std::unique_ptr<GameState> m_pendingState;
    bool m_running = true;
    Uint64 m_lastTicks = 0;
};
