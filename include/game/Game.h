#pragma once

#include <SDL2/SDL.h>
#include <memory>
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

class GameState;

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

    // Wipes and rebuilds the per-race world (cars, rocks, gates, player
    // assignment, HUD state) and reenters the Countdown state.
    void resetRace();

    RaceSession& race() { return *m_race; }
    const RaceSession& race() const { return *m_race; }
    void synchronizeEngineSound();

    // -- Rendering helpers ------------------------------------------------
    // Rebuilds any car's lap texture whose lap count changed. Called every frame.
    void refreshLapTextures();
    // Rebuilds player1/2 label textures using the currently-driven cars' colors.
    void rebuildPlayerLabels();

    SDL_TexturePtr makeLabelTexture(const char* text, SDL_Color color, SDL_Rect& outRect);

    // -- Public shared data (owned by Game, accessed by states) -----------
    AppWindow app;
    int windowWidth = 0;
    int windowHeight = 0;

    std::unique_ptr<Track> track;
    std::unique_ptr<StartLine> startLine;
    SDL_TexturePtr carTexture;

    // Player 1/2 labels in the top-left corner.
    SDL_TexturePtr player1LabelTexture;
    SDL_TexturePtr player2LabelTexture;
    SDL_Rect player1LabelRect{ 20, 20, 0, 0 };
    SDL_Rect player2LabelRect{ 20, 20, 0, 0 };

    // Per-car lap HUD stacked on the right, one row per car (indexed the same
    // as RaceSession::cars()). Drawn by HudRenderer::renderLeaderboard.
    std::vector<CarHudRow> carHud;

    // Countdown overlay.
    float countdownTimer = 0.0f;
    SDL_TexturePtr countdownTexture;
    SDL_Rect countdownRect{ 0, 0, 0, 0 };
    int countdownLastDigit = -2;

    // Race outcome presentation.
    SDL_TexturePtr winnerTexture;
    SDL_Rect winnerRect{ 0, 0, 0, 0 };
    SDL_TexturePtr winnerHintTexture;
    SDL_Rect winnerHintRect{ 0, 0, 0, 0 };

    // Audio backends.
    EngineSound engineSound;
    UiSound uiSound;
    Voice voice;
    bool voiceAvailable = false;
    std::unique_ptr<Announcer> announcer;

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
