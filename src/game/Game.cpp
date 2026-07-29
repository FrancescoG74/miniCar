#include "game/Game.h"

#include <SDL2/SDL_ttf.h>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>

#include "CollisionSystem.h"
#include "game/GamePhases.h"
#include "game/GameState.h"

namespace {

void drawCircle(SDL_Renderer* renderer, float cx, float cy, float radius, SDL_Color color) {
    constexpr int kPoints = 24;
    SDL_FPoint points[kPoints + 1];
    for (int i = 0; i <= kPoints; ++i) {
        float t = static_cast<float>(i) / kPoints * 2.0f * static_cast<float>(M_PI);
        points[i] = { cx + radius * std::cos(t), cy + radius * std::sin(t) };
    }
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawLinesF(renderer, points, kPoints + 1);
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

    engineSound.init(); // silent no-op on failure
    uiSound.init();
    voiceAvailable = voice.init();

    resetRace();
    return true;
}

int Game::run() {
    m_lastTicks = SDL_GetTicks64();
    while (m_running) {
        pollEvents();
        if (!m_running) break;

        Uint64 nowTicks = SDL_GetTicks64();
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

void Game::resetRace() {
    // Wipe any HUD textures that reference the previous race.
    player1LabelTexture.reset();
    player2LabelTexture.reset();
    for (auto& tex : carLapTextures) tex.reset();
    winnerTexture.reset();
    winnerHintTexture.reset();
    countdownTexture.reset();

    cars = Car::createInitialGrid(kLaneOffset);
    rocks = Rock::createInitialRocks(*track);
    gates = Gate::createInitialGates(*track, kTrackWidth);

    player1Index = Car::assignPlayer1(cars, app.window);
    player2Index = -1;

    rebuildPlayerLabels();

    carLapTextures.clear();
    carLapTextures.resize(cars.size());
    carLapRects.assign(cars.size(), SDL_Rect{ 0, 0, 0, 0 });
    carLastLaps.assign(cars.size(), -1);

    winnerIndex = -1;
    winnerRect = SDL_Rect{ 0, 0, 0, 0 };
    winnerHintRect = SDL_Rect{ 0, 0, 0, 0 };

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
    player1LabelRect = SDL_Rect{ 20, 20, 0, 0 };
    if (player1Index >= 0) {
        player1LabelTexture = makeLabelTexture("Player 1",
                                                cars[player1Index].getColor(), player1LabelRect);
    } else {
        player1LabelTexture.reset();
    }
    player2LabelRect = SDL_Rect{ 20, player1LabelRect.y + player1LabelRect.h + 14, 0, 0 };
    if (player2Index >= 0) {
        player2LabelTexture = makeLabelTexture("Player 2",
                                                cars[player2Index].getColor(), player2LabelRect);
        player2LabelRect.y = player1LabelRect.y + player1LabelRect.h + 14;
    } else {
        player2LabelTexture.reset();
    }
}

void Game::refreshLapTextures() {
    for (size_t i = 0; i < cars.size(); ++i) {
        if (cars[i].laps == carLastLaps[i]) continue;
        std::string text = std::string(cars[i].getName()) + ": Lap " +
                            std::to_string(cars[i].laps);
        carLapTextures[i] = makeLabelTexture(text.c_str(), cars[i].getColor(), carLapRects[i]);
        carLastLaps[i] = cars[i].laps;
    }
}

SDL_TexturePtr Game::makeLabelTexture(const char* text, SDL_Color color, SDL_Rect& outRect) {
    if (!app.font) return SDL_TexturePtr{};
    SDL_Surface* surface = TTF_RenderText_Blended(app.font, text, color);
    if (!surface) return SDL_TexturePtr{};
    SDL_Texture* texture = SDL_CreateTextureFromSurface(app.renderer, surface);
    outRect.w = surface->w;
    outRect.h = surface->h;
    SDL_FreeSurface(surface);
    return SDL_TexturePtr(texture);
}

// -- Event loop ----------------------------------------------------------

void Game::pollEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            m_running = false;
            continue;
        }

        if (event.type == SDL_KEYDOWN) {
            SDL_Keycode k = event.key.keysym.sym;

            // Universal keys handled by Game, not by any state.
            if (k == SDLK_ESCAPE) {
                m_running = false;
                continue;
            }
            if (k == SDLK_r) {
                resetRace();
                continue;
            }

            // Player 2 join/leave is only meaningful mid-race (not when finished).
            const bool finished = dynamic_cast<FinishedState*>(m_state.get()) != nullptr;
            if (k == SDLK_2 && player2Index < 0 && !finished) {
                int newP2 = Car::assignPlayer2(cars, app.window, player1Index);
                if (newP2 >= 0) {
                    player2Index = newP2;
                    rebuildPlayerLabels();
                    carLastLaps[player2Index] = -1; // driver color changed; rebuild HUD row
                }
                continue;
            }
            if (k == SDLK_1 && player2Index >= 0 && !finished) {
                Car::removePlayer2(cars, app.window, player1Index, player2Index);
                int oldP2 = player2Index;
                player2Index = -1;
                rebuildPlayerLabels();
                if (oldP2 >= 0 && oldP2 < static_cast<int>(carLastLaps.size())) {
                    carLastLaps[oldP2] = -1;
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

    renderWorld();

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    renderPlayerLabels();
    renderLeaderboard();

    if (m_state) m_state->renderOverlay(*this);

    SDL_RenderPresent(renderer);
}

void Game::renderWorld() {
    SDL_Renderer* renderer = app.renderer;
    track->render(renderer);
    startLine->render(renderer);

    for (const auto& rock : rocks) rock.render(renderer);
    for (const auto& gate : gates) gate.render(renderer);

    for (const auto& car : cars) {
        TrackPoint p = track->sample(car.s); // sampled once for the rotation angle
        float cx = car.getPosition().x;
        float cy = car.getPosition().y;

        SDL_SetTextureColorMod(carTexture.get(), car.getColor().r, car.getColor().g, car.getColor().b);
        SDL_Rect dst{
            static_cast<int>(cx - kCarWidth / 2.0f),
            static_cast<int>(cy - kCarHeight / 2.0f),
            kCarWidth,
            kCarHeight
        };
        double angleDeg = p.angle * 180.0 / M_PI + 90.0;
        SDL_RenderCopyEx(renderer, carTexture.get(), nullptr, &dst, angleDeg, nullptr, SDL_FLIP_NONE);

        if (car.playerNumber != 0) {
            drawCircle(renderer, cx, cy, kCarHeight * 0.75f, SDL_Color{ 255, 255, 255, 255 });
        }
    }
}

void Game::renderPlayerLabels() {
    SDL_Renderer* renderer = app.renderer;
    if (player1LabelTexture) {
        SDL_Rect background{
            player1LabelRect.x - 8, player1LabelRect.y - 6,
            player1LabelRect.w + 16, player1LabelRect.h + 12
        };
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
        SDL_RenderFillRect(renderer, &background);
        SDL_RenderCopy(renderer, player1LabelTexture.get(), nullptr, &player1LabelRect);
    }
    if (player2LabelTexture) {
        SDL_Rect background{
            player2LabelRect.x - 8, player2LabelRect.y - 6,
            player2LabelRect.w + 16, player2LabelRect.h + 12
        };
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
        SDL_RenderFillRect(renderer, &background);
        SDL_RenderCopy(renderer, player2LabelTexture.get(), nullptr, &player2LabelRect);
    }
}

void Game::renderLeaderboard() {
    SDL_Renderer* renderer = app.renderer;

    // Sort by lap count (then distance) so the current leader is on top.
    std::vector<size_t> order(cars.size());
    for (size_t i = 0; i < order.size(); ++i) order[i] = i;
    std::sort(order.begin(), order.end(), [&](size_t a, size_t b) {
        if (cars[a].laps != cars[b].laps) return cars[a].laps > cars[b].laps;
        return cars[a].distanceTraveled > cars[b].distanceTraveled;
    });

    int rowY = 20;
    for (size_t idx : order) {
        SDL_Texture* tex = carLapTextures[idx].get();
        if (!tex) continue;
        SDL_Rect& r = carLapRects[idx];
        r.x = windowWidth - 20 - r.w;
        r.y = rowY;
        SDL_Rect background{ r.x - 8, r.y - 4, r.w + 16, r.h + 8 };
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
        SDL_RenderFillRect(renderer, &background);
        SDL_RenderCopy(renderer, tex, nullptr, &r);
        rowY += r.h + 8;
    }
}
