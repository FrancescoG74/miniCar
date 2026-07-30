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
    for (size_t i = 0; i < game.race().cars().size(); ++i) {
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
            (game.windowWidth - static_cast<int>(static_cast<float>(naturalRect.w) * kCountdownScale)) / 2,
            (game.windowHeight - static_cast<int>(static_cast<float>(naturalRect.h) * kCountdownScale)) / 2,
            static_cast<int>(static_cast<float>(naturalRect.w) * kCountdownScale),
            static_cast<int>(static_cast<float>(naturalRect.h) * kCountdownScale)
        };
        game.countdownLastDigit = digit;

        // Audible cue delegated to the pre-selected Announcer strategy
        // (VoiceAnnouncer / BeepAnnouncer / SilentAnnouncer null-object).
        if (game.announcer) game.announcer->countdown(digit);
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
    const Uint8* keys = SDL_GetKeyboardState(nullptr);
    game.race().update(dt, keys);
    game.synchronizeEngineSound();
    game.refreshLapTextures();

    if (game.race().winnerIndex()) {
        game.transitionTo(std::make_unique<FinishedState>());
    }
}

// ==========================================================================
// FinishedState
// ==========================================================================

void FinishedState::onEnter(Game& game) {
    const auto& winnerIndex = game.race().winnerIndex();
    if (!winnerIndex) return;
    const auto& cars = game.race().cars();
    std::string winMsg = std::string(cars[*winnerIndex].getName()) + " wins the race!";
    game.winnerTexture = game.makeLabelTexture(winMsg.c_str(),
                                                cars[*winnerIndex].getColor(),
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
    for (size_t i = 0; i < game.race().cars().size(); ++i) {
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
