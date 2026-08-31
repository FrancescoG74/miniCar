#include "game/GamePhases.h"

#include <SDL3/SDL.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <string>

#include "CollisionSystem.h"
#include "game/Game.h"
#include "input/KeyboardInput.h"

// ==========================================================================
// MenuState
// ==========================================================================

void MenuState::onEnter(Game& game) {
    SDL_StartTextInput(game.app.window);
    m_focus = Field::Players;
    m_dirty = true;
    m_lapsFresh = true;

    m_titleTexture = game.makeLabelTexture("miniCar - Race Setup",
                                            SDL_Color{ 255, 255, 255, 255 }, m_titleRect);
    m_instructionsTexture = game.makeLabelTexture(
        "Up/Down: select field    Left/Right or digits: change laps/players    "
        "Type: edit name    Enter: start    Esc: quit",
        SDL_Color{ 200, 200, 200, 255 }, m_instructionsRect);
}

void MenuState::onExit(Game& game) {
    SDL_StopTextInput(game.app.window);
}

void MenuState::moveFocus(Game& game, int direction) {
    int index = static_cast<int>(m_focus);
    for (int step = 0; step < kFieldCount; ++step) {
        index = (index + direction + kFieldCount) % kFieldCount;
        auto candidate = static_cast<Field>(index);
        // Name fields for AI-controlled slots aren't editable; skip over them.
        if (candidate == Field::Player1Name && game.raceSetup.playerCount < 1) continue;
        if (candidate == Field::Player2Name && game.raceSetup.playerCount < 2) continue;
        m_focus = candidate;
        if (candidate == Field::Laps) m_lapsFresh = true;
        break;
    }
    m_dirty = true;
}

void MenuState::adjustValue(Game& game, int direction) {
    switch (m_focus) {
    case Field::Players:
        game.raceSetup.playerCount = std::clamp(game.raceSetup.playerCount + direction, 0, 2);
        break;
    case Field::Laps:
        game.raceSetup.laps = std::clamp(game.raceSetup.laps + direction, kMinLaps, kMaxLaps);
        m_lapsFresh = true; // next typed digit should replace, not append to, this value
        break;
    default:
        break;
    }
    m_dirty = true;
}

void MenuState::appendText(Game& game, const char* utf8Text) {
    if (m_focus == Field::Laps) {
        // Only single ASCII digits are meaningful for a lap count.
        if (utf8Text[0] < '0' || utf8Text[0] > '9' || utf8Text[1] != '\0') return;
        int digit = utf8Text[0] - '0';
        int typed = m_lapsFresh ? digit : game.raceSetup.laps * 10 + digit;
        game.raceSetup.laps = std::clamp(typed, kMinLaps, kMaxLaps);
        m_lapsFresh = false;
        m_dirty = true;
        return;
    }

    std::string* target = nullptr;
    if (m_focus == Field::Player1Name && game.raceSetup.playerCount >= 1) {
        target = &game.raceSetup.player1Name;
    } else if (m_focus == Field::Player2Name && game.raceSetup.playerCount >= 2) {
        target = &game.raceSetup.player2Name;
    }
    if (!target || target->size() >= kMaxNameLength) return;

    *target += utf8Text;
    m_dirty = true;
}

void MenuState::backspace(Game& game) {
    if (m_focus == Field::Laps) {
        game.raceSetup.laps = std::max(kMinLaps, game.raceSetup.laps / 10);
        m_lapsFresh = true;
        m_dirty = true;
        return;
    }

    std::string* target = nullptr;
    if (m_focus == Field::Player1Name) target = &game.raceSetup.player1Name;
    else if (m_focus == Field::Player2Name) target = &game.raceSetup.player2Name;
    if (!target || target->empty()) return;

    target->pop_back();
    m_dirty = true;
}

void MenuState::handleEvent(Game& game, const SDL_Event& event) {
    if (event.type == SDL_EVENT_TEXT_INPUT) {
        appendText(game, event.text.text);
        return;
    }
    if (event.type != SDL_EVENT_KEY_DOWN) return;

    switch (event.key.key) {
    case SDLK_ESCAPE:
    case SDLK_AC_BACK: // Android back button/gesture
        game.quit();
        break;
    case SDLK_UP:
        moveFocus(game, -1);
        break;
    case SDLK_DOWN:
    case SDLK_TAB:
        moveFocus(game, 1);
        break;
    case SDLK_LEFT:
    case SDLK_MINUS:
    case SDLK_KP_MINUS:
        adjustValue(game, -1);
        break;
    case SDLK_RIGHT:
    case SDLK_PLUS:
    case SDLK_EQUALS:
    case SDLK_KP_PLUS:
        adjustValue(game, 1);
        break;
    case SDLK_BACKSPACE:
        backspace(game);
        break;
    case SDLK_RETURN:
    case SDLK_KP_ENTER:
        game.startRace();
        break;
    default:
        break;
    }
}

void MenuState::update(Game& game, float /*dt*/) {
    if (!m_dirty) return;
    rebuildTextures(game);
    m_dirty = false;
}

void MenuState::rebuildTextures(Game& game) {
    auto buildLine = [&](Field field, const std::string& text) {
        bool focused = (m_focus == field);
        SDL_Color color = focused ? SDL_Color{ 255, 220, 60, 255 } : SDL_Color{ 255, 255, 255, 255 };
        std::string prefixed = (focused ? "> " : "  ") + text;
        Line& line = m_lines[static_cast<int>(field)];
        line.texture = game.makeLabelTexture(prefixed.c_str(), color, line.rect);
    };

    buildLine(Field::Players,
              "Players: " + std::to_string(game.raceSetup.playerCount) +
                  "  (0 = all AI, 2 = two humans)");

    buildLine(Field::Player1Name,
              "Player 1 name: " + (game.raceSetup.playerCount >= 1
                                       ? (game.raceSetup.player1Name.empty() ? "(auto)"
                                                                              : game.raceSetup.player1Name)
                                       : std::string("(AI controlled)")));

    buildLine(Field::Player2Name,
              "Player 2 name: " + (game.raceSetup.playerCount >= 2
                                       ? (game.raceSetup.player2Name.empty() ? "(auto)"
                                                                              : game.raceSetup.player2Name)
                                       : std::string("(AI controlled)")));

    buildLine(Field::Laps, "Laps to win: " + std::to_string(game.raceSetup.laps));
}

void MenuState::renderOverlay(Game& game) {
    SDL_Renderer* renderer = game.app.renderer;

    SDL_FRect full{ 0, 0, static_cast<float>(game.windowWidth), static_cast<float>(game.windowHeight) };
    SDL_SetRenderDrawColor(renderer, 24, 28, 36, 255);
    SDL_RenderFillRect(renderer, &full);

    auto drawCentered = [&](SDL_Texture* texture, SDL_FRect& rect, int lineY) {
        if (!texture) return;
        rect.x = (game.windowWidth - rect.w) / 2.0f;
        rect.y = static_cast<float>(lineY);
        SDL_RenderTexture(renderer, texture, nullptr, &rect);
    };

    int y = game.windowHeight / 2 - 160;
    drawCentered(m_titleTexture.get(), m_titleRect, y);
    y += m_titleRect.h + 40;

    for (int i = 0; i < kFieldCount; ++i) {
        drawCentered(m_lines[i].texture.get(), m_lines[i].rect, y);
        y += m_lines[i].rect.h + 20;
    }

    y += 20;
    drawCentered(m_instructionsTexture.get(), m_instructionsRect, y);
}

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

    // Engines idle silently -- no way to jump the start. Still call
    // synchronizeEngineSound() every frame so the stream keeps getting fed
    // (and any residual revs from a previous race decay smoothly to idle).
    for (size_t i = 0; i < game.race().cars().size(); ++i) {
        game.engineSound.setCarSpeed(static_cast<int>(i), 0.0f);
    }
    game.engineSound.update(dt);

    // Rebuild the number texture and cue audio each time the displayed digit
    // changes (once per second, effectively).
    int digit = std::max(0, static_cast<int>(std::ceil(game.countdownTimer)) - 1);
    if (digit != game.countdownLastDigit) {
        SDL_FRect naturalRect{ 0, 0, 0, 0 };
        game.countdownTexture = game.makeLabelTexture(std::to_string(digit).c_str(),
                                                       SDL_Color{ 255, 255, 255, 255 }, naturalRect);
        constexpr float kCountdownScale = 4.0f;
        game.countdownRect = SDL_FRect{
            (static_cast<float>(game.windowWidth) - naturalRect.w * kCountdownScale) / 2.0f,
            (static_cast<float>(game.windowHeight) - naturalRect.h * kCountdownScale) / 2.0f,
            naturalRect.w * kCountdownScale,
            naturalRect.h * kCountdownScale
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
    SDL_FRect background{
        game.countdownRect.x - 20, game.countdownRect.y - 12,
        game.countdownRect.w + 40, game.countdownRect.h + 24
    };
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
    SDL_RenderFillRect(renderer, &background);
    SDL_RenderTexture(renderer, game.countdownTexture.get(), nullptr, &game.countdownRect);
}

// ==========================================================================
// RacingState
// ==========================================================================

void RacingState::update(Game& game, float dt) {
    const bool* boolKeys = SDL_GetKeyboardState(nullptr);
    // Convert bool* to Uint8* for RaceSession API (SDL_SCANCODE values are 0-based indices)
    // This is safe because we're just using the array as a key state lookup table
    const Uint8* keys = reinterpret_cast<const Uint8*>(boolKeys);

    // Fold the virtual on-screen buttons into a copy of the keyboard state so
    // Player 1's existing WASD KeyboardInput drives them identically, whether
    // the accelerate/brake/steer signal came from a physical key or a finger.
    std::array<Uint8, SDL_SCANCODE_COUNT> combinedKeys{};
    if (game.touchControls.enabled) {
        std::memcpy(combinedKeys.data(), keys, combinedKeys.size());
        const KeyboardInput::Scheme scheme = KeyboardInput::wasd();
        if (game.touchControls.accelerate()) combinedKeys[scheme.accelerate] = 1;
        if (game.touchControls.brake()) combinedKeys[scheme.brake] = 1;
        if (game.touchControls.left()) combinedKeys[scheme.left] = 1;
        if (game.touchControls.right()) combinedKeys[scheme.right] = 1;
        keys = combinedKeys.data();
    }

    game.race().update(dt, keys);
    game.synchronizeEngineSound(dt);
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
    game.winnerRect.x = (static_cast<float>(game.windowWidth) - game.winnerRect.w) / 2.0f;
    game.winnerRect.y = (static_cast<float>(game.windowHeight) - game.winnerRect.h) / 2.0f;

    game.winnerHintTexture = game.makeLabelTexture("Press Esc to quit",
                                                    SDL_Color{ 230, 230, 230, 255 },
                                                    game.winnerHintRect);
    game.winnerHintRect.x = (static_cast<float>(game.windowWidth) - game.winnerHintRect.w) / 2.0f;
    game.winnerHintRect.y = game.winnerRect.y + game.winnerRect.h + 16.0f;
}

void FinishedState::update(Game& game, float dt) {
    for (size_t i = 0; i < game.race().cars().size(); ++i) {
        game.engineSound.setCarSpeed(static_cast<int>(i), 0.0f);
    }
    game.engineSound.update(dt);
    game.refreshLapTextures();
}

void FinishedState::renderOverlay(Game& game) {
    SDL_Renderer* renderer = game.app.renderer;

    SDL_FRect overlay{ 0, 0, static_cast<float>(game.windowWidth), static_cast<float>(game.windowHeight) };
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 140);
    SDL_RenderFillRect(renderer, &overlay);

    if (game.winnerTexture) {
        SDL_FRect background{
            game.winnerRect.x - 20, game.winnerRect.y - 14,
            game.winnerRect.w + 40, game.winnerRect.h + 28
        };
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
        SDL_RenderFillRect(renderer, &background);
        SDL_RenderTexture(renderer, game.winnerTexture.get(), nullptr, &game.winnerRect);
    }
    if (game.winnerHintTexture) {
        SDL_RenderTexture(renderer, game.winnerHintTexture.get(), nullptr, &game.winnerHintRect);
    }
}
