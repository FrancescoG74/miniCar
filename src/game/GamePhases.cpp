#include "game/GamePhases.h"

#include <SDL2/SDL.h>
#include <algorithm>
#include <cmath>
#include <string>

#include "CollisionSystem.h"
#include "game/Game.h"

// ==========================================================================
// CountdownState
// ==========================================================================

void CountdownState::onEnter(Game& game) {
    game.countdownTimer = Game::kCountdownDuration;
    game.countdownLastDigit = -2;
    for (int i = 0; i < EngineSound::kMaxCars; ++i) {
        game.engineSound.setCarSpeed(i, 0.0f);
    }
}

void CountdownState::update(Game& game, float dt) {
    game.countdownTimer = std::max(0.0f, game.countdownTimer - dt);

    // Engines idle silently -- no way to jump the start.
    for (size_t i = 0; i < game.cars.size(); ++i) {
        game.engineSound.setCarSpeed(static_cast<int>(i), 0.0f);
    }

    // Rebuild the number texture and cue audio each time the displayed digit
    // changes (once per second, effectively).
    int digit = std::max(0, static_cast<int>(std::ceil(game.countdownTimer)) - 1);
    if (digit != game.countdownLastDigit) {
        SDL_Rect naturalRect{ 0, 0, 0, 0 };
        game.countdownTexture = game.makeLabelTexture(std::to_string(digit).c_str(),
                                                       SDL_Color{ 255, 255, 255, 255 }, naturalRect);
        constexpr float kCountdownScale = 4.0f;
        game.countdownRect = SDL_Rect{
            (game.windowWidth - static_cast<int>(naturalRect.w * kCountdownScale)) / 2,
            (game.windowHeight - static_cast<int>(naturalRect.h * kCountdownScale)) / 2,
            static_cast<int>(naturalRect.w * kCountdownScale),
            static_cast<int>(naturalRect.h * kCountdownScale)
        };
        game.countdownLastDigit = digit;

        // Audible cue: spoken via espeak-ng when available, otherwise a plain
        // beep for 3/2/1 and a higher, longer beep for 0 (the "go" moment).
        if (game.voiceAvailable) {
            game.voice.speak(digit > 0 ? std::to_string(digit) : std::string("Go"));
        } else if (digit > 0) {
            game.uiSound.playBeep(440.0f, 0.15f);
        } else {
            game.uiSound.playBeep(880.0f, 0.3f);
        }
    }

    // Keep lap textures live so the leaderboard shows on the first frame.
    game.refreshLapTextures();

    if (game.countdownTimer <= 0.0f) {
        game.transitionTo(std::make_unique<RacingState>());
    }
}

void CountdownState::renderOverlay(Game& game) {
    if (!game.countdownTexture) return;
    SDL_Renderer* renderer = game.app.renderer;
    SDL_Rect background{
        game.countdownRect.x - 20, game.countdownRect.y - 12,
        game.countdownRect.w + 40, game.countdownRect.h + 24
    };
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
    SDL_RenderFillRect(renderer, &background);
    SDL_RenderCopy(renderer, game.countdownTexture.get(), nullptr, &game.countdownRect);
}

// ==========================================================================
// RacingState
// ==========================================================================

void RacingState::update(Game& game, float dt) {
    for (auto& gate : game.gates) gate.update(dt);

    // Rebuild the list of s positions AI cars should slow down for
    // (only currently-closed gates block the track).
    std::vector<float> blockedS;
    for (const auto& gate : game.gates) {
        if (gate.isClosed()) blockedS.push_back(gate.s);
    }

    const Uint8* keys = SDL_GetKeyboardState(nullptr);
    CarControls ctrl;
    ctrl.keys = keys;
    ctrl.maxSpeed = Game::kMaxCarSpeed;
    ctrl.playerAccel = Game::kPlayerAccel;
    ctrl.playerBrake = Game::kPlayerBrake;
    ctrl.playerSteerRate = Game::kPlayerSteerRate;
    ctrl.laneLimit = Game::kLaneLimit;
    ctrl.aiAccel = Game::kAiAccel;
    ctrl.recoveryBoost = Game::kRecoveryBoost;
    ctrl.blockedSPositions = &blockedS;
    ctrl.track = game.track.get();

    const float totalLength = game.track->totalLength();
    for (size_t i = 0; i < game.cars.size(); ++i) {
        Car& car = game.cars[i];
        car.update(dt, game.cars, i, totalLength, ctrl);
        game.engineSound.setCarSpeed(static_cast<int>(i), car.speed / Game::kMaxCarSpeed);

        if (car.laps >= Game::kLapsToWin && game.winnerIndex < 0) {
            game.winnerIndex = static_cast<int>(i);
        }
    }

    CollisionSystem::Config cfg{ totalLength,
                                  static_cast<float>(Game::kCarHeight),
                                  static_cast<float>(Game::kCarWidth) };
    CollisionSystem::resolveAll(game.cars, game.rocks, game.gates, cfg);

    game.refreshLapTextures();

    if (game.winnerIndex >= 0) {
        game.transitionTo(std::make_unique<FinishedState>());
    }
}

// ==========================================================================
// FinishedState
// ==========================================================================

void FinishedState::onEnter(Game& game) {
    if (game.winnerIndex < 0) return;
    std::string winMsg = std::string(game.cars[game.winnerIndex].getName()) + " wins the race!";
    game.winnerTexture = game.makeLabelTexture(winMsg.c_str(),
                                                game.cars[game.winnerIndex].getColor(),
                                                game.winnerRect);
    game.winnerRect.x = (game.windowWidth - game.winnerRect.w) / 2;
    game.winnerRect.y = (game.windowHeight - game.winnerRect.h) / 2;

    game.winnerHintTexture = game.makeLabelTexture("Press Esc to quit",
                                                    SDL_Color{ 230, 230, 230, 255 },
                                                    game.winnerHintRect);
    game.winnerHintRect.x = (game.windowWidth - game.winnerHintRect.w) / 2;
    game.winnerHintRect.y = game.winnerRect.y + game.winnerRect.h + 16;
}

void FinishedState::update(Game& game, float /*dt*/) {
    for (size_t i = 0; i < game.cars.size(); ++i) {
        game.engineSound.setCarSpeed(static_cast<int>(i), 0.0f);
    }
    game.refreshLapTextures();
}

void FinishedState::renderOverlay(Game& game) {
    SDL_Renderer* renderer = game.app.renderer;

    SDL_Rect overlay{ 0, 0, game.windowWidth, game.windowHeight };
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 140);
    SDL_RenderFillRect(renderer, &overlay);

    if (game.winnerTexture) {
        SDL_Rect background{
            game.winnerRect.x - 20, game.winnerRect.y - 14,
            game.winnerRect.w + 40, game.winnerRect.h + 28
        };
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
        SDL_RenderFillRect(renderer, &background);
        SDL_RenderCopy(renderer, game.winnerTexture.get(), nullptr, &game.winnerRect);
    }
    if (game.winnerHintTexture) {
        SDL_RenderCopy(renderer, game.winnerHintTexture.get(), nullptr, &game.winnerHintRect);
    }
}
