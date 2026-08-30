#include "game/Game.h"

#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <iostream>
#include <string>

#include "game/GamePhases.h"
#include "game/GameState.h"
#include "game/WorldRenderer.h"

namespace {

// Loads the 5 rock sprite variants shipped in assets/. Missing/failed loads
// leave that slot null; Rock::render falls back to the procedural polygon.
std::array<SDL_TexturePtr, 5> loadRockTextures(SDL_Renderer* renderer) {
    std::array<SDL_TexturePtr, 5> textures;
    for (std::size_t i = 0; i < textures.size(); ++i) {
        std::string path = std::string(MINICAR_ASSETS_DIR) + "/rock" + std::to_string(i + 1) + ".png";
        SDL_Texture* texture = IMG_LoadTexture(renderer, path.c_str());
        if (!texture) {
            std::cerr << "IMG_LoadTexture failed for " << path << ": " << SDL_GetError() << std::endl;
            continue;
        }
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
        textures[i] = SDL_TexturePtr(texture);
    }
    return textures;
}

} // namespace

Game::Game() = default;
Game::~Game() = default;

bool Game::init(int width, int height, const char* title) {
    windowWidth = width;
    windowHeight = height;

    if (!initApp(app, width, height, title)) return false;

    track = std::make_unique<Track>(kTrackCenterX, kTrackCenterY,
                                     kTrackStraight, kTrackRadius, kTrackWidth);
    startLine = std::make_unique<StartLine>(*track, kTrackWidth);
    carTexture = Car::createTexture(app.renderer, kCarWidth, kCarHeight);
    rockTextures = loadRockTextures(app.renderer);
    m_raceTuning.laneLimit = kTrackWidth / 2.0f - 12.0f;
    m_raceTuning.carLength = static_cast<float>(kCarHeight);
    m_raceTuning.carWidth = static_cast<float>(kCarWidth);
    m_aiTuning.homeLaneOffset = 25.0f;
    m_race = std::make_unique<RaceSession>(*track, kTrackWidth, m_raceTuning, m_aiTuning);

    engineSound.init(); // silent no-op on failure
    uiSound.init();
    voiceAvailable = voice.init();
    announcer = Announcer::create(&voice, &uiSound, voiceAvailable);

    touchControls.layout(windowWidth, windowHeight);
    // Belt-and-suspenders: always on for Android builds, and also auto-detect
    // a touch device on any other platform (e.g. a touchscreen laptop/tablet).
#ifdef ANDROID
    touchControls.enabled = true;
#endif
    int touchDeviceCount = 0;
    SDL_TouchID* touchDevices = SDL_GetTouchDevices(&touchDeviceCount);
    if (touchDevices) SDL_free(touchDevices);
    if (touchDeviceCount > 0) touchControls.enabled = true;

    raceSetup.laps = m_raceTuning.lapsToWin;
    showMenu();
    return true;
}

int Game::run() {
    m_lastTicks = SDL_GetTicks();
    while (m_running) {
        pollEvents();
        if (!m_running) break;

        Uint64 nowTicks = SDL_GetTicks();
        float dt = static_cast<float>(nowTicks - m_lastTicks) / 1000.0f;
        m_lastTicks = nowTicks;
        if (dt > 0.05f) dt = 0.05f; // avoid large jumps after pause/resize

        if (m_state) m_state->update(*this, dt);
        applyPendingState();

        renderFrame();
    }
    return 0;
}

void Game::transitionTo(std::unique_ptr<GameState> next) {
    m_pendingState = std::move(next);
}

void Game::applyPendingState() {
    if (!m_pendingState) return;
    if (m_state) m_state->onExit(*this);
    m_state = std::move(m_pendingState);
    if (m_state) m_state->onEnter(*this);
}

void Game::showMenu() {
    transitionTo(std::make_unique<MenuState>());
    applyPendingState();
}

void Game::startRace() {
    // Wipe any HUD textures that reference the previous race.
    player1LabelTexture.reset();
    player2LabelTexture.reset();
    carHud.clear();
    winnerTexture.reset();
    winnerHintTexture.reset();
    countdownTexture.reset();

    m_raceTuning.lapsToWin = std::clamp(raceSetup.laps, 1, 99);
    m_race->setLapsToWin(m_raceTuning.lapsToWin);
    m_race->reset(std::clamp(raceSetup.playerCount, 0, 2));

    // Custom names live in `raceSetup` for the rest of the race's lifetime, so
    // the char* Actor::name stashes below stay valid until the next reset.
    if (const auto& player1Index = m_race->player1Index()) {
        if (!raceSetup.player1Name.empty()) {
            m_race->cars()[*player1Index].setName(raceSetup.player1Name.c_str());
        }
        std::cout << "Player 1 controls the " << m_race->cars()[*player1Index].getName()
                  << " car (W/A/S/D).\n";
    }
    if (const auto& player2Index = m_race->player2Index()) {
        if (!raceSetup.player2Name.empty()) {
            m_race->cars()[*player2Index].setName(raceSetup.player2Name.c_str());
        }
        std::cout << "Player 2 controls the " << m_race->cars()[*player2Index].getName()
                  << " car (I/J/K/L).\n";
    } else if (m_race->player1Index()) {
        std::cout << "Press '2' to add Player 2, '1' to remove them." << std::endl;
    }
    updateWindowTitle();

    rebuildPlayerLabels();

    carHud.resize(m_race->cars().size());

    winnerRect = SDL_FRect{ 0, 0, 0, 0 };
    winnerHintRect = SDL_FRect{ 0, 0, 0, 0 };

    countdownTimer = kCountdownDuration;
    countdownLastDigit = -2;

    for (int i = 0; i < EngineSound::kMaxCars; ++i) {
        engineSound.setCarSpeed(i, 0.0f);
    }

    // The Countdown phase owns the pre-race freeze; entering it sets the timer
    // and any first-frame cues via onEnter().
    transitionTo(std::make_unique<CountdownState>());
    applyPendingState();
}

void Game::rebuildPlayerLabels() {
    player1LabelRect = SDL_FRect{ 20, 20, 0, 0 };
    if (const auto& player1Index = m_race->player1Index()) {
        player1LabelTexture = makeLabelTexture("Player 1",
                                                m_race->cars()[*player1Index].getColor(), player1LabelRect);
    } else {
        player1LabelTexture.reset();
    }
    player2LabelRect = SDL_FRect{ 20, player1LabelRect.y + player1LabelRect.h + 14, 0, 0 };
    if (const auto& player2Index = m_race->player2Index()) {
        player2LabelTexture = makeLabelTexture("Player 2",
                                                m_race->cars()[*player2Index].getColor(), player2LabelRect);
        player2LabelRect.y = player1LabelRect.y + player1LabelRect.h + 14;
    } else {
        player2LabelTexture.reset();
    }
}

void Game::refreshLapTextures() {
    const auto& cars = m_race->cars();
    for (size_t i = 0; i < cars.size(); ++i) {
        if (cars[i].laps == carHud[i].lastLaps) continue;
        std::string text = std::string(cars[i].getName()) + ": Lap " +
                            std::to_string(cars[i].laps);
        carHud[i].texture = makeLabelTexture(text.c_str(), cars[i].getColor(), carHud[i].rect);
        carHud[i].lastLaps = cars[i].laps;
    }
}

SDL_TexturePtr Game::makeLabelTexture(const char* text, SDL_Color color, SDL_FRect& outRect) {
    if (!app.font) return SDL_TexturePtr{};
    // SDL3 TTF_RenderText_Blended takes text length as a parameter
    SDL_Surface* surface = TTF_RenderText_Blended(app.font, text, std::strlen(text), color);
    if (!surface) return SDL_TexturePtr{};
    SDL_Texture* texture = SDL_CreateTextureFromSurface(app.renderer, surface);
    outRect.w = static_cast<float>(surface->w);
    outRect.h = static_cast<float>(surface->h);
    SDL_DestroySurface(surface);
    return SDL_TexturePtr(texture);
}

// -- Event loop ----------------------------------------------------------

void Game::pollEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            m_running = false;
            continue;
        }

        // Route finger events to the virtual controller regardless of game
        // state; RacingState is the only one that reads the resulting button
        // states, but hit-testing here keeps the logic in one place.
        if (event.type == SDL_EVENT_FINGER_DOWN || event.type == SDL_EVENT_FINGER_MOTION ||
            event.type == SDL_EVENT_FINGER_UP || event.type == SDL_EVENT_FINGER_CANCELED) {
            SDL_Event converted = event;
            SDL_ConvertEventToRenderCoordinates(app.renderer, &converted);
            touchControls.handleEvent(converted);
        }

        if (event.type == SDL_EVENT_KEY_DOWN && !(m_state && m_state->ownsInput())) {
            SDL_Keycode k = event.key.key;

            // Universal keys handled by Game, not by any state. Suppressed
            // while a state (e.g. the menu) wants to own all keyboard input.
            // SDLK_AC_BACK is the Android hardware/gesture back button.
            if (k == SDLK_ESCAPE || k == SDLK_AC_BACK) {
                m_running = false;
                continue;
            }
            if (k == SDLK_R) {
                showMenu();
                continue;
            }

            if (k == SDLK_2 && !m_race->player2Index()) {
                auto newP2 = m_race->joinPlayer2();
                if (newP2) {
                    std::cout << "Player 2 joined: controls the "
                              << m_race->cars()[*newP2].getName() << " car (I/J/K/L)." << std::endl;
                    updateWindowTitle();
                    rebuildPlayerLabels();
                    carHud[*newP2].lastLaps = -1; // driver color changed; rebuild HUD row
                }
                continue;
            }
            if (k == SDLK_1 && m_race->player2Index()) {
                const std::size_t oldP2 = *m_race->player2Index();
                if (m_race->removePlayer2()) {
                    std::cout << "Player 2 left; " << m_race->cars()[oldP2].getName()
                              << " car returns to AI control." << std::endl;
                    updateWindowTitle();
                    rebuildPlayerLabels();
                    if (oldP2 < carHud.size()) {
                        carHud[oldP2].lastLaps = -1;
                    }
                }
                continue;
            }
        }

        if (m_state) m_state->handleEvent(*this, event);
    }
}

// -- Rendering -----------------------------------------------------------

void Game::renderFrame() {
    SDL_Renderer* renderer = app.renderer;

    // Grass background.
    SDL_SetRenderDrawColor(renderer, 34, 120, 50, 255);
    SDL_RenderClear(renderer);

    if (!m_state || m_state->rendersWorld()) {
        WorldRenderer::CarSprite carSprite{ carTexture.get(), kCarWidth, kCarHeight };
        std::array<SDL_Texture*, 5> rockSprites{
            rockTextures[0].get(), rockTextures[1].get(), rockTextures[2].get(),
            rockTextures[3].get(), rockTextures[4].get(),
        };
        WorldRenderer::render(renderer, *track, *startLine, *m_race, carSprite, rockSprites);

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        HudRenderer::renderLabel(renderer, player1LabelTexture.get(), player1LabelRect);
        HudRenderer::renderLabel(renderer, player2LabelTexture.get(), player2LabelRect);
        HudRenderer::renderLeaderboard(renderer, windowWidth, m_race->cars(), carHud);

        // Drawn whenever the world is visible (Countdown/Racing/Finished) so
        // the player can see where the buttons are even before the race
        // starts, not just during RacingState.
        touchControls.render(renderer);
    }

    if (m_state) m_state->renderOverlay(*this);

    SDL_RenderPresent(renderer);
}

void Game::synchronizeEngineSound() {
    const auto& cars = m_race->cars();
    for (std::size_t index = 0; index < cars.size(); ++index) {
        engineSound.setCarSpeed(static_cast<int>(index),
                                cars[index].speed / m_race->tuning().maxCarSpeed);
    }
}

void Game::updateWindowTitle() {
    const auto& cars = m_race->cars();
    const auto& player1Index = m_race->player1Index();
    const auto& player2Index = m_race->player2Index();
    if (!app.window || !player1Index) return;

    std::string title = "miniCar - P1: ";
    title += cars[*player1Index].getName();
    title += " (WASD)";
    if (player2Index) {
        title += "  |  P2: ";
        title += cars[*player2Index].getName();
        title += " (IJKL)";
    } else {
        title += "  |  press 2 to add P2";
    }
    SDL_SetWindowTitle(app.window, title.c_str());
}
