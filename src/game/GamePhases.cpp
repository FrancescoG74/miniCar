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

namespace {

bool pointInRect(float x, float y, const SDL_FRect& rect) {
    return x >= rect.x && x <= rect.x + rect.w &&
           y >= rect.y && y <= rect.y + rect.h;
}

} // namespace

// ==========================================================================
// MenuState
// ==========================================================================

void MenuState::onEnter(Game& game) {
    SDL_StartTextInput(game.app.window);
    m_focus = Field::Players;
    m_dirty = true;
    m_lapsFresh = true;
    m_showTrackGallery = false;

    m_titleTexture = game.makeLabelTexture("miniCar - Race Setup",
                                            SDL_Color{ 255, 255, 255, 255 }, m_titleRect);
    m_instructionsTexture = game.makeLabelTexture(
        "Up/Down: select field    Left/Right or digits: change laps/players    "
        "Type: edit name    T: tracks    Enter: start    Esc: quit",
        SDL_Color{ 200, 200, 200, 255 }, m_instructionsRect);
    m_galleryTitleTexture = game.makeLabelTexture(
        "Track Gallery - Races cycle through these ten layouts",
        SDL_Color{ 255, 255, 255, 255 }, m_galleryTitleRect);
    m_galleryHintTexture = game.makeLabelTexture(
        "Esc: back to race setup",
        SDL_Color{ 200, 200, 200, 255 }, m_galleryHintRect);
    m_touchTitleTexture = game.makeLabelTexture(
        "RACE SETUP", SDL_Color{ 255, 255, 255, 255 }, m_touchTitleRect);
    m_touchPlayersHintTexture = game.makeLabelTexture(
        "Choose how many people are playing",
        SDL_Color{ 200, 210, 225, 255 }, m_touchPlayersHintRect);
    m_touchLapHintTexture = game.makeLabelTexture(
        "LAPS TO WIN", SDL_Color{ 200, 210, 225, 255 }, m_touchLapHintRect);
    m_touchStartTexture = game.makeLabelTexture(
        "START RACE", SDL_Color{ 255, 255, 255, 255 }, m_touchStartRect);
    constexpr const char* playerChoices[] = { "AI ONLY", "1 PLAYER", "2 PLAYERS" };
    for (std::size_t i = 0; i < m_touchPlayerTextures.size(); ++i) {
        m_touchPlayerTextures[i] = game.makeLabelTexture(
            playerChoices[i], SDL_Color{ 255, 255, 255, 255 }, m_touchPlayerRects[i]);
    }
    m_touchMinusTexture = game.makeLabelTexture(
        "-", SDL_Color{ 255, 255, 255, 255 }, m_touchMinusRect);
    m_touchPlusTexture = game.makeLabelTexture(
        "+", SDL_Color{ 255, 255, 255, 255 }, m_touchPlusRect);
    m_touchLapValueTexture = game.makeLabelTexture(
        std::to_string(game.raceSetup.laps).c_str(),
        SDL_Color{ 255, 255, 255, 255 }, m_touchLapValueRect);

    for (std::size_t i = 0; i < TrackGenerator::kTrackCount; ++i) {
        const std::string number = i < 9 ? "0" + std::to_string(i + 1)
                                         : std::to_string(i + 1);
        const std::string label = number + "  " + TrackGenerator::typeAt(i).name;
        m_trackLabels[i] = game.makeLabelTexture(
            label.c_str(), SDL_Color{ 230, 230, 230, 255 }, m_trackLabelRects[i]);
    }
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
    const bool touchMenu = game.touchControls.enabled;
    if (touchMenu && !m_showTrackGallery &&
        (event.type == SDL_EVENT_FINGER_DOWN || event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)) {
        float x = event.type == SDL_EVENT_FINGER_DOWN ? event.tfinger.x : event.button.x;
        float y = event.type == SDL_EVENT_FINGER_DOWN ? event.tfinger.y : event.button.y;
        const auto width = static_cast<float>(game.windowWidth);
        const auto height = static_cast<float>(game.windowHeight);
        const float cardWidth = std::min(width * 0.27f, 340.0f);
        const float cardGap = std::max(12.0f, width * 0.025f);
        const float cardsWidth = cardWidth * 3.0f + cardGap * 2.0f;
        const float cardStartX = (width - cardsWidth) * 0.5f;
        const float cardY = height * 0.39f;
        const float cardHeight = std::clamp(height * 0.19f, 104.0f, 160.0f);

        for (int players = 0; players <= 2; ++players) {
            SDL_FRect card{ cardStartX + static_cast<float>(players) * (cardWidth + cardGap), cardY,
                            cardWidth, cardHeight };
            if (pointInRect(x, y, card)) {
                game.raceSetup.playerCount = players;
                m_dirty = true;
                return;
            }
        }

        const float controlSize = std::clamp(height * 0.12f, 72.0f, 104.0f);
        const float controlsY = height * 0.68f;
        const float centerX = width * 0.5f;
        SDL_FRect minus{ centerX - controlSize * 1.8f, controlsY,
                         controlSize, controlSize };
        SDL_FRect plus{ centerX + controlSize * 0.8f, controlsY,
                        controlSize, controlSize };
        if (pointInRect(x, y, minus)) {
            game.raceSetup.laps = std::max(kMinLaps, game.raceSetup.laps - 1);
            m_dirty = true;
            return;
        }
        if (pointInRect(x, y, plus)) {
            game.raceSetup.laps = std::min(kMaxLaps, game.raceSetup.laps + 1);
            m_dirty = true;
            return;
        }

        SDL_FRect start{ width * 0.25f, height * 0.84f, width * 0.5f,
                         std::clamp(height * 0.11f, 72.0f, 100.0f) };
        if (pointInRect(x, y, start)) {
            game.startRace();
            return;
        }
    }

    if (event.type == SDL_EVENT_TEXT_INPUT) {
        if (!m_showTrackGallery) appendText(game, event.text.text);
        return;
    }
    if (event.type != SDL_EVENT_KEY_DOWN) return;

    switch (event.key.key) {
    case SDLK_ESCAPE:
    case SDLK_AC_BACK: // Android back button/gesture
        if (m_showTrackGallery) {
            m_showTrackGallery = false;
        } else {
            game.quit();
        }
        break;
    case SDLK_T:
        m_showTrackGallery = true;
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

    m_touchLapValueTexture = game.makeLabelTexture(
        std::to_string(game.raceSetup.laps).c_str(),
        SDL_Color{ 255, 255, 255, 255 }, m_touchLapValueRect);
}

void MenuState::renderOverlay(Game& game) {
    SDL_Renderer* renderer = game.app.renderer;

    SDL_FRect full{ 0, 0, static_cast<float>(game.windowWidth), static_cast<float>(game.windowHeight) };
    SDL_SetRenderDrawColor(renderer, 24, 28, 36, 255);
    SDL_RenderFillRect(renderer, &full);

    if (m_showTrackGallery) {
        m_galleryTitleRect.x = (static_cast<float>(game.windowWidth) - m_galleryTitleRect.w) / 2.0f;
        m_galleryTitleRect.y = 24.0f;
        SDL_RenderTexture(renderer, m_galleryTitleTexture.get(), nullptr, &m_galleryTitleRect);

        const float cellWidth = static_cast<float>(game.windowWidth) / 5.0f;
        const float galleryTop = 72.0f;
        const float galleryBottom = 48.0f;
        const float rowHeight = (static_cast<float>(game.windowHeight) - galleryTop - galleryBottom) / 2.0f;

        for (std::size_t i = 0; i < TrackGenerator::kTrackCount; ++i) {
            const auto& type = TrackGenerator::typeAt(i);
            const float trackWidth = Game::kTrackWidth;
            const float radialExtent = 1.0f + std::abs(type.lobeA) +
                                       std::abs(type.lobeB) + std::abs(type.lobeC);
            const float scaleX = cellWidth * 0.68f /
                (2.0f * type.radiusX * radialExtent + trackWidth);
            const float scaleY = rowHeight * 0.44f /
                (2.0f * type.radiusY * radialExtent + trackWidth);
            const float scale = std::min(scaleX, scaleY);
            const int column = static_cast<int>(i % 5);
            const int row = static_cast<int>(i / 5);
            const float centerX = cellWidth * (static_cast<float>(column) + 0.5f);
            const float rowTop = galleryTop + rowHeight * static_cast<float>(row);
            const float centerY = rowTop + rowHeight * 0.43f;

            Track preview = TrackGenerator::createPreview(i, centerX, centerY, trackWidth, scale);
            preview.render(renderer);

            SDL_FRect& labelRect = m_trackLabelRects[i];
            labelRect.x = centerX - labelRect.w / 2.0f;
            labelRect.y = rowTop + rowHeight * 0.79f;
            SDL_RenderTexture(renderer, m_trackLabels[i].get(), nullptr, &labelRect);
        }

        m_galleryHintRect.x = (static_cast<float>(game.windowWidth) - m_galleryHintRect.w) / 2.0f;
        m_galleryHintRect.y = static_cast<float>(game.windowHeight) - m_galleryHintRect.h - 12.0f;
        SDL_RenderTexture(renderer, m_galleryHintTexture.get(), nullptr, &m_galleryHintRect);
        return;
    }

    if (game.touchControls.enabled) {
        const auto width = static_cast<float>(game.windowWidth);
        const auto height = static_cast<float>(game.windowHeight);
        SDL_SetRenderDrawColor(renderer, 24, 28, 36, 255);
        SDL_FRect panel{ width * 0.08f, height * 0.08f, width * 0.84f, height * 0.84f };
        SDL_SetRenderDrawColor(renderer, 34, 40, 52, 255);
        SDL_RenderFillRect(renderer, &panel);

        auto drawCentered = [&](SDL_Texture* texture, const SDL_FRect& naturalRect,
                                float centerX, float centerY) {
            if (!texture) return;
            SDL_FRect rect{ centerX - naturalRect.w * 0.5f,
                            centerY - naturalRect.h * 0.5f,
                            naturalRect.w, naturalRect.h };
            SDL_RenderTexture(renderer, texture, nullptr, &rect);
        };
        drawCentered(m_touchTitleTexture.get(), m_touchTitleRect, width * 0.5f, height * 0.18f);
        drawCentered(m_touchPlayersHintTexture.get(), m_touchPlayersHintRect,
                     width * 0.5f, height * 0.29f);

        const float cardWidth = std::min(width * 0.27f, 340.0f);
        const float cardGap = std::max(12.0f, width * 0.025f);
        const float cardsWidth = cardWidth * 3.0f + cardGap * 2.0f;
        const float cardStartX = (width - cardsWidth) * 0.5f;
        const float cardY = height * 0.39f;
        const float cardHeight = std::clamp(height * 0.19f, 104.0f, 160.0f);
        for (int players = 0; players <= 2; ++players) {
            SDL_FRect card{ cardStartX + static_cast<float>(players) * (cardWidth + cardGap), cardY,
                            cardWidth, cardHeight };
            const bool selected = game.raceSetup.playerCount == players;
            SDL_SetRenderDrawColor(renderer, selected ? 45 : 52,
                                   selected ? 115 : 62, selected ? 190 : 76, 255);
            SDL_RenderFillRect(renderer, &card);
            SDL_SetRenderDrawColor(renderer, selected ? 255 : 115,
                                   selected ? 220 : 125, selected ? 90 : 140, 255);
            SDL_RenderRect(renderer, &card);
            drawCentered(m_touchPlayerTextures[players].get(), m_touchPlayerRects[players],
                         card.x + card.w * 0.5f, card.y + card.h * 0.5f);
        }

        drawCentered(m_touchLapHintTexture.get(), m_touchLapHintRect,
                     width * 0.5f, height * 0.63f);
        const float controlSize = std::clamp(height * 0.12f, 72.0f, 104.0f);
        const float controlsY = height * 0.68f;
        const float centerX = width * 0.5f;
        SDL_FRect minus{ centerX - controlSize * 1.8f, controlsY,
                         controlSize, controlSize };
        SDL_FRect plus{ centerX + controlSize * 0.8f, controlsY,
                        controlSize, controlSize };
        SDL_FRect lapValue{ centerX - controlSize * 0.5f, controlsY,
                            controlSize, controlSize };
        SDL_FRect controls[] = { minus, plus };
        for (const auto& control : controls) {
            SDL_SetRenderDrawColor(renderer, 58, 68, 84, 255);
            SDL_RenderFillRect(renderer, &control);
            SDL_SetRenderDrawColor(renderer, 150, 165, 185, 255);
            SDL_RenderRect(renderer, &control);
        }
        drawCentered(m_touchMinusTexture.get(), m_touchMinusRect,
                     minus.x + minus.w * 0.5f, minus.y + minus.h * 0.5f);
        drawCentered(m_touchLapValueTexture.get(), m_touchLapValueRect,
                     lapValue.x + lapValue.w * 0.5f, lapValue.y + lapValue.h * 0.5f);
        drawCentered(m_touchPlusTexture.get(), m_touchPlusRect,
                     plus.x + plus.w * 0.5f, plus.y + plus.h * 0.5f);

        SDL_FRect start{ width * 0.25f, height * 0.84f, width * 0.5f,
                         std::clamp(height * 0.11f, 72.0f, 100.0f) };
        SDL_SetRenderDrawColor(renderer, 35, 150, 82, 255);
        SDL_RenderFillRect(renderer, &start);
        SDL_SetRenderDrawColor(renderer, 120, 235, 155, 255);
        SDL_RenderRect(renderer, &start);
        drawCentered(m_touchStartTexture.get(), m_touchStartRect,
                     start.x + start.w * 0.5f, start.y + start.h * 0.5f);

        return;
    }

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

    game.winnerHintTexture = game.makeLabelTexture("Enter: next race    R: race setup    Esc: quit",
                                                    SDL_Color{ 230, 230, 230, 255 },
                                                    game.winnerHintRect);
    game.winnerHintRect.x = (static_cast<float>(game.windowWidth) - game.winnerHintRect.w) / 2.0f;
    game.winnerHintRect.y = game.winnerRect.y + game.winnerRect.h + 16.0f;

    if (game.touchControls.enabled) {
        m_nextRaceButtonRect = SDL_FRect{
            static_cast<float>(game.windowWidth) * 0.30f,
            static_cast<float>(game.windowHeight) * 0.72f,
            static_cast<float>(game.windowWidth) * 0.40f,
            std::clamp(static_cast<float>(game.windowHeight) * 0.12f, 96.0f, 120.0f)
        };
        m_nextRaceButtonTexture = game.makeLabelTexture(
            "NEXT RACE", SDL_Color{ 255, 255, 255, 255 }, m_nextRaceButtonTextRect);
        game.winnerHintTexture = game.makeLabelTexture(
            "Tap Next Race to play again",
            SDL_Color{ 230, 230, 230, 255 }, game.winnerHintRect);
        game.winnerHintRect.x =
            (static_cast<float>(game.windowWidth) - game.winnerHintRect.w) * 0.5f;
    }
}

void FinishedState::handleEvent(Game& game, const SDL_Event& event) {
    if (event.type == SDL_EVENT_KEY_DOWN &&
        (event.key.key == SDLK_RETURN || event.key.key == SDLK_KP_ENTER)) {
        game.startRace();
        return;
    }

    if (game.touchControls.enabled &&
        (event.type == SDL_EVENT_FINGER_DOWN || event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)) {
        const float x = event.type == SDL_EVENT_FINGER_DOWN ? event.tfinger.x : event.button.x;
        const float y = event.type == SDL_EVENT_FINGER_DOWN ? event.tfinger.y : event.button.y;
        const bool inside = x >= m_nextRaceButtonRect.x &&
                            x <= m_nextRaceButtonRect.x + m_nextRaceButtonRect.w &&
                            y >= m_nextRaceButtonRect.y &&
                            y <= m_nextRaceButtonRect.y + m_nextRaceButtonRect.h;
        if (inside) game.startRace();
    }
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
    if (game.touchControls.enabled) {
        SDL_SetRenderDrawColor(renderer, 35, 150, 82, 255);
        SDL_RenderFillRect(renderer, &m_nextRaceButtonRect);
        SDL_SetRenderDrawColor(renderer, 120, 235, 155, 255);
        SDL_RenderRect(renderer, &m_nextRaceButtonRect);
        if (m_nextRaceButtonTexture) {
            SDL_FRect textRect{
                m_nextRaceButtonRect.x + (m_nextRaceButtonRect.w - m_nextRaceButtonTextRect.w) * 0.5f,
                m_nextRaceButtonRect.y + (m_nextRaceButtonRect.h - m_nextRaceButtonTextRect.h) * 0.5f,
                m_nextRaceButtonTextRect.w,
                m_nextRaceButtonTextRect.h
            };
            SDL_RenderTexture(renderer, m_nextRaceButtonTexture.get(), nullptr, &textRect);
        }
    }
}
