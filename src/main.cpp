#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include "Audio.h"
#include "actor/Car.h"
#include "actor/Gate.h"
#include "actor/Rock.h"
#include "Setup.h"
#include "actor/StartLine.h"
#include "Track.h"
#include "Voice.h"

namespace {

SDL_Texture* makeLabelTexture(SDL_Renderer* renderer, TTF_Font* font, const char* text,
                               SDL_Color color, SDL_Rect& outRect) {
    if (!font) return nullptr;
    SDL_Surface* surface = TTF_RenderText_Blended(font, text, color);
    if (!surface) return nullptr;
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    outRect.w = surface->w;
    outRect.h = surface->h;
    SDL_FreeSurface(surface);
    return texture;
}

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

int main(int argc, char* argv[]) {
    const int windowWidth = 1600;
    const int windowHeight = 900;
    AppWindow app;
    if (!initApp(app, windowWidth, windowHeight, "miniCar")) return 1;
    SDL_Window* window = app.window;
    SDL_Renderer* renderer = app.renderer;
    TTF_Font* font = app.font;

    // Circuit laid out in the middle of the window.
    const float trackWidth = 130.0f;
    Track track(800.0f, 450.0f, /*straightLength=*/500.0f, /*radius=*/190.0f, /*width=*/trackWidth);

    const int carWidth = 16;
    const int carHeight = 30;
    SDL_Texture* carTexture = Car::createTexture(renderer, carWidth, carHeight);

    const float laneOffset = 25.0f;
    std::vector<Car> cars;
    std::vector<Rock> rocks;
    std::vector<Gate> gates;
    StartLine startLine(track, trackWidth);

    int player1Index = -1;
    int player2Index = -1; // -1 = single-player; set when player 2 joins

    // "Player 1" / "Player 2" labels rendered once and blitted in the top-left corner every frame.
    SDL_Rect player1LabelRect{ 20, 20, 0, 0 };
    SDL_Rect player2LabelRect{ 20, 20, 0, 0 };
    SDL_Texture* player1LabelTexture = nullptr;
    SDL_Texture* player2LabelTexture = nullptr; // created lazily when P2 joins

    // Per-car lap HUD stacked in the top-right corner. Each entry is refreshed only
    // when that car's lap count (or driver color) changes, so we don't rebuild
    // textures every frame.
    std::vector<SDL_Texture*> carLapTextures;
    std::vector<SDL_Rect> carLapRects;
    std::vector<int> carLastLaps;

    const int kLapsToWin = 5;
    bool raceFinished = false;
    int winnerIndex = -1;
    SDL_Texture* winnerTexture = nullptr;
    SDL_Rect winnerRect{ 0, 0, 0, 0 };
    SDL_Texture* winnerHintTexture = nullptr;
    SDL_Rect winnerHintRect{ 0, 0, 0, 0 };

    // Pre-race countdown: cars/gates stay frozen while this counts down from
    // kCountdownDuration to 0, showing "3", "2", "1", "0" before the race begins.
    const float kCountdownDuration = 4.0f;
    float countdownTimer = 0.0f;
    SDL_Texture* countdownTexture = nullptr;
    SDL_Rect countdownRect{ 0, 0, 0, 0 };
    int countdownLastDigit = -2; // sentinel so the first frame always builds a texture

    EngineSound engineSound;
    engineSound.init(); // if this fails, the app still runs (silently)
    UiSound uiSound;
    uiSound.init(); // if this fails, the app still runs (silently)
    Voice voice;
    const bool voiceAvailable = voice.init(); // false if espeak-ng wasn't found/available
    const float maxCarSpeed = 130.0f;
    const float playerAccel = 60.0f;
    const float playerBrake = 90.0f;
    const float playerSteerRate = 70.0f;
    const float laneLimit = trackWidth / 2.0f - 12.0f;
    const float aiAccel = 50.0f; // how fast AI cars regain their cruising speed after a collision
    const float kRecoveryBoost = 2.5f; // extra acceleration multiplier right after a crash

    // (Re)initializes everything that changes race-to-race: the car grid, player
    // assignment, HUD textures and win state. Called once up front and again
    // whenever the player presses 'R' to restart.
    auto resetRace = [&]() {
        if (player1LabelTexture) { SDL_DestroyTexture(player1LabelTexture); player1LabelTexture = nullptr; }
        if (player2LabelTexture) { SDL_DestroyTexture(player2LabelTexture); player2LabelTexture = nullptr; }
        for (SDL_Texture*& tex : carLapTextures) {
            if (tex) SDL_DestroyTexture(tex);
        }
        if (winnerTexture) { SDL_DestroyTexture(winnerTexture); winnerTexture = nullptr; }
        if (winnerHintTexture) { SDL_DestroyTexture(winnerHintTexture); winnerHintTexture = nullptr; }
        if (countdownTexture) { SDL_DestroyTexture(countdownTexture); countdownTexture = nullptr; }

        cars = Car::createInitialGrid(laneOffset);
        rocks = Rock::createInitialRocks(track);
        gates = Gate::createInitialGates(track, trackWidth);

        player1Index = Car::assignPlayer1(cars, window);
        player2Index = -1;

        player1LabelRect = SDL_Rect{ 20, 20, 0, 0 };
        player1LabelTexture = makeLabelTexture(renderer, font, "Player 1",
                                                 cars[player1Index].getColor(), player1LabelRect);
        player2LabelRect = SDL_Rect{ 20, player1LabelRect.y + player1LabelRect.h + 14, 0, 0 };

        carLapTextures.assign(cars.size(), nullptr);
        carLapRects.assign(cars.size(), SDL_Rect{ 0, 0, 0, 0 });
        carLastLaps.assign(cars.size(), -1);

        raceFinished = false;
        winnerIndex = -1;
        winnerRect = SDL_Rect{ 0, 0, 0, 0 };
        winnerHintRect = SDL_Rect{ 0, 0, 0, 0 };

        countdownTimer = kCountdownDuration;
        countdownLastDigit = -2;

        for (int i = 0; i < EngineSound::kMaxCars; ++i) {
            engineSound.setCarSpeed(i, 0.0f);
        }
    };

    resetRace();

    bool running = true;
    SDL_Event event;
    Uint64 lastTicks = SDL_GetTicks64();

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_KEYDOWN) {
                SDL_Keycode k = event.key.keysym.sym;
                if (k == SDLK_ESCAPE) {
                    running = false;
                } else if (k == SDLK_r) {
                    resetRace();
                } else if (k == SDLK_2 && player2Index < 0 && !raceFinished) {
                    // Add player 2: pick a random AI car.
                    int newP2 = Car::assignPlayer2(cars, window, player1Index);
                    if (newP2 >= 0) {
                        player2Index = newP2;
                        player2LabelTexture = makeLabelTexture(renderer, font, "Player 2",
                                                                 cars[player2Index].getColor(), player2LabelRect);
                        player2LabelRect.y = player1LabelRect.y + player1LabelRect.h + 14;
                        carLastLaps[player2Index] = -1; // driver color changed, rebuild that car's HUD row
                    }
                } else if (k == SDLK_1 && player2Index >= 0 && !raceFinished) {
                    // Remove player 2: hand the car back to the AI.
                    Car::removePlayer2(cars, window, player1Index, player2Index);
                    player2Index = -1;
                    if (player2LabelTexture) {
                        SDL_DestroyTexture(player2LabelTexture);
                        player2LabelTexture = nullptr;
                    }
                    for (size_t i = 0; i < carLastLaps.size(); ++i) carLastLaps[i] = -1;
                }
            }
        }

        Uint64 nowTicks = SDL_GetTicks64();
        float dt = static_cast<float>(nowTicks - lastTicks) / 1000.0f;
        lastTicks = nowTicks;
        if (dt > 0.05f) dt = 0.05f; // avoid large jumps (e.g. after a pause/resize)

        const Uint8* keys = SDL_GetKeyboardState(nullptr);

        const float totalLength = track.totalLength();
        if (countdownTimer > 0.0f) {
            // Freeze the world during the pre-race countdown; engines idle silently
            // and no car/gate update runs, so there's no way to jump the start.
            countdownTimer = std::max(0.0f, countdownTimer - dt);
            for (size_t i = 0; i < cars.size(); ++i) {
                engineSound.setCarSpeed(static_cast<int>(i), 0.0f);
            }
        } else if (!raceFinished) {
            for (auto& gate : gates) gate.update(dt);

            // Rebuild the list of s positions AI cars should slow down for (only
            // currently-closed gates block the track).
            std::vector<float> blockedS;
            for (const auto& gate : gates) {
                if (gate.isClosed()) blockedS.push_back(gate.s);
            }

            CarControls ctrl;
            ctrl.keys = keys;
            ctrl.maxSpeed = maxCarSpeed;
            ctrl.playerAccel = playerAccel;
            ctrl.playerBrake = playerBrake;
            ctrl.playerSteerRate = playerSteerRate;
            ctrl.laneLimit = laneLimit;
            ctrl.aiAccel = aiAccel;
            ctrl.recoveryBoost = kRecoveryBoost;
            ctrl.blockedSPositions = &blockedS;
            ctrl.track = &track;

            for (size_t i = 0; i < cars.size(); ++i) {
                Car& car = cars[i];
                car.update(dt, cars, i, totalLength, ctrl);
                engineSound.setCarSpeed(static_cast<int>(i), car.speed / maxCarSpeed);

                if (car.laps >= kLapsToWin && !raceFinished) {
                    raceFinished = true;
                    winnerIndex = static_cast<int>(i);

                    std::string winMsg = std::string(cars[winnerIndex].getName()) + " wins the race!";
                    winnerTexture = makeLabelTexture(renderer, font, winMsg.c_str(), cars[winnerIndex].getColor(), winnerRect);
                    winnerRect.x = (windowWidth - winnerRect.w) / 2;
                    winnerRect.y = (windowHeight - winnerRect.h) / 2;

                    winnerHintTexture = makeLabelTexture(renderer, font, "Press Esc to quit",
                                                          SDL_Color{ 230, 230, 230, 255 }, winnerHintRect);
                    winnerHintRect.x = (windowWidth - winnerHintRect.w) / 2;
                    winnerHintRect.y = winnerRect.y + winnerRect.h + 16;
                }
            }

            Car::resolveCollisions(cars, totalLength, carHeight, carWidth);
            Rock::resolveCarCollisions(cars, rocks, totalLength, carHeight, carWidth);
            Gate::resolveCarCollisions(cars, gates, totalLength, carHeight);
        } else {
            for (size_t i = 0; i < cars.size(); ++i) {
                engineSound.setCarSpeed(static_cast<int>(i), 0.0f);
            }
        }

        // Rebuild any car's lap texture whose lap count changed (or that was reset
        // due to a driver change). Textures are colored by the car's current color
        // so player cars stand out with their bright hues.
        for (size_t i = 0; i < cars.size(); ++i) {
            if (cars[i].laps == carLastLaps[i]) continue;
            if (carLapTextures[i]) SDL_DestroyTexture(carLapTextures[i]);
            std::string text = std::string(cars[i].getName()) + ": Lap " +
                                std::to_string(cars[i].laps);
            carLapTextures[i] = makeLabelTexture(renderer, font, text.c_str(),
                                                    cars[i].getColor(), carLapRects[i]);
            carLastLaps[i] = cars[i].laps;
        }

        // Rebuild the countdown number texture only when the displayed digit changes.
        if (countdownTimer > 0.0f) {
            int digit = std::max(0, static_cast<int>(std::ceil(countdownTimer)) - 1);
            if (digit != countdownLastDigit) {
                if (countdownTexture) SDL_DestroyTexture(countdownTexture);
                SDL_Rect naturalRect{ 0, 0, 0, 0 };
                countdownTexture = makeLabelTexture(renderer, font, std::to_string(digit).c_str(),
                                                      SDL_Color{ 255, 255, 255, 255 }, naturalRect);
                constexpr float kCountdownScale = 4.0f;
                countdownRect = SDL_Rect{
                    (windowWidth - static_cast<int>(naturalRect.w * kCountdownScale)) / 2,
                    (windowHeight - static_cast<int>(naturalRect.h * kCountdownScale)) / 2,
                    static_cast<int>(naturalRect.w * kCountdownScale),
                    static_cast<int>(naturalRect.h * kCountdownScale)
                };
                countdownLastDigit = digit;

                // Audible cue for each number: spoken via espeak-ng when available
                // (see Voice/MINICAR_HAVE_ESPEAK), otherwise a plain beep for 3/2/1
                // and a higher, longer beep for 0 (the "go" moment).
                if (voiceAvailable) {
                    voice.speak(digit > 0 ? std::to_string(digit) : std::string("Go"));
                } else if (digit > 0) {
                    uiSound.playBeep(440.0f, 0.15f);
                } else {
                    uiSound.playBeep(880.0f, 0.3f);
                }
            }
        }

        // Grass background.
        SDL_SetRenderDrawColor(renderer, 34, 120, 50, 255);
        SDL_RenderClear(renderer);

        track.render(renderer);
        startLine.render(renderer);

        for (const auto& rock : rocks) {
            rock.render(renderer);
        }
        for (const auto& gate : gates) {
            gate.render(renderer);
        }

        for (const auto& car : cars) {
            TrackPoint p = track.sample(car.s); // sampled once for the rotation angle only
            float cx = car.getPosition().x;
            float cy = car.getPosition().y;

            SDL_SetTextureColorMod(carTexture, car.getColor().r, car.getColor().g, car.getColor().b);
            SDL_Rect dst{
                static_cast<int>(cx - carWidth / 2.0f),
                static_cast<int>(cy - carHeight / 2.0f),
                carWidth,
                carHeight
            };
            double angleDeg = p.angle * 180.0 / M_PI + 90.0;
            SDL_RenderCopyEx(renderer, carTexture, nullptr, &dst, angleDeg, nullptr, SDL_FLIP_NONE);

            if (car.playerNumber != 0) {
                drawCircle(renderer, cx, cy, carHeight * 0.75f, SDL_Color{ 255, 255, 255, 255 });
            }
        }

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        if (player1LabelTexture) {
            SDL_Rect background{
                player1LabelRect.x - 8, player1LabelRect.y - 6,
                player1LabelRect.w + 16, player1LabelRect.h + 12
            };
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
            SDL_RenderFillRect(renderer, &background);
            SDL_RenderCopy(renderer, player1LabelTexture, nullptr, &player1LabelRect);
        }
        if (player2LabelTexture) {
            SDL_Rect background{
                player2LabelRect.x - 8, player2LabelRect.y - 6,
                player2LabelRect.w + 16, player2LabelRect.h + 12
            };
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
            SDL_RenderFillRect(renderer, &background);
            SDL_RenderCopy(renderer, player2LabelTexture, nullptr, &player2LabelRect);
        }

        // Big countdown number centered on screen while the race hasn't started yet.
        if (countdownTimer > 0.0f && countdownTexture) {
            SDL_Rect background{
                countdownRect.x - 20, countdownRect.y - 12,
                countdownRect.w + 40, countdownRect.h + 24
            };
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
            SDL_RenderFillRect(renderer, &background);
            SDL_RenderCopy(renderer, countdownTexture, nullptr, &countdownRect);
        }

        // All-cars leaderboard on the right edge, sorted by lap count (and total
        // distance as a tie-breaker) so the current leader appears on top.
        {
            std::vector<size_t> order(cars.size());
            for (size_t i = 0; i < order.size(); ++i) order[i] = i;
            std::sort(order.begin(), order.end(), [&](size_t a, size_t b) {
                if (cars[a].laps != cars[b].laps) return cars[a].laps > cars[b].laps;
                return cars[a].distanceTraveled > cars[b].distanceTraveled;
            });

            int rowY = 20;
            for (size_t idx : order) {
                SDL_Texture* tex = carLapTextures[idx];
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

        if (raceFinished) {
            SDL_Rect overlay{ 0, 0, windowWidth, windowHeight };
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 140);
            SDL_RenderFillRect(renderer, &overlay);

            if (winnerTexture) {
                SDL_Rect background{
                    winnerRect.x - 20, winnerRect.y - 14,
                    winnerRect.w + 40, winnerRect.h + 28
                };
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
                SDL_RenderFillRect(renderer, &background);
                SDL_RenderCopy(renderer, winnerTexture, nullptr, &winnerRect);
            }
            if (winnerHintTexture) {
                SDL_RenderCopy(renderer, winnerHintTexture, nullptr, &winnerHintRect);
            }
        }

        SDL_RenderPresent(renderer);
    }

    if (player1LabelTexture) SDL_DestroyTexture(player1LabelTexture);
    if (player2LabelTexture) SDL_DestroyTexture(player2LabelTexture);
    for (SDL_Texture* tex : carLapTextures) {
        if (tex) SDL_DestroyTexture(tex);
    }
    if (winnerTexture) SDL_DestroyTexture(winnerTexture);
    if (winnerHintTexture) SDL_DestroyTexture(winnerHintTexture);
    if (countdownTexture) SDL_DestroyTexture(countdownTexture);
    engineSound.shutdown();
    uiSound.shutdown();
    voice.shutdown();
    SDL_DestroyTexture(carTexture);
    shutdownApp(app);

    return 0;
}
