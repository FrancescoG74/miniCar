#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include "Audio.h"
#include "Car.h"
#include "Gate.h"
#include "Rock.h"
#include "Setup.h"
#include "Track.h"

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
    const int windowWidth = 1280;
    const int windowHeight = 960;
    AppWindow app;
    if (!initApp(app, windowWidth, windowHeight, "miniCar")) return 1;
    SDL_Window* window = app.window;
    SDL_Renderer* renderer = app.renderer;
    TTF_Font* font = app.font;

    // Circuit laid out in the middle of the window.
    const float trackWidth = 130.0f;
    Track track(640.0f, 480.0f, /*straightLength=*/500.0f, /*radius=*/190.0f, /*width=*/trackWidth);

    const int carWidth = 16;
    const int carHeight = 30;
    SDL_Texture* carTexture = Car::createTexture(renderer, carWidth, carHeight);

    const float laneOffset = 25.0f;
    std::vector<Car> cars = Car::createInitialGrid(laneOffset);
    std::vector<Rock> rocks = Rock::createInitialRocks(track);
    std::vector<Gate> gates = Gate::createInitialGates(track, trackWidth);

    int player1Index = Car::assignPlayer1(cars, window);
    int player2Index = -1; // -1 = single-player; set when player 2 joins

    // "Player 1" / "Player 2" labels rendered once and blitted in the top-left corner every frame.
    SDL_Rect player1LabelRect{ 20, 20, 0, 0 };
    SDL_Rect player2LabelRect{ 20, 20, 0, 0 };
    SDL_Texture* player1LabelTexture = makeLabelTexture(renderer, font, "Player 1", cars[player1Index].getColor(), player1LabelRect);
    SDL_Texture* player2LabelTexture = nullptr; // created lazily when P2 joins
    player2LabelRect.y = player1LabelRect.y + player1LabelRect.h + 14;

    // Small lap-count HUD in the top-right corner, refreshed only when a lap count changes.
    SDL_Texture* player1LapTexture = nullptr;
    SDL_Texture* player2LapTexture = nullptr;
    SDL_Rect player1LapRect{ 0, 20, 0, 0 };
    SDL_Rect player2LapRect{ 0, 20, 0, 0 };
    int player1LastLaps = -1;
    int player2LastLaps = -1;

    const int kLapsToWin = 5;
    bool raceFinished = false;
    int winnerIndex = -1;
    SDL_Texture* winnerTexture = nullptr;
    SDL_Rect winnerRect{ 0, 0, 0, 0 };
    SDL_Texture* winnerHintTexture = nullptr;
    SDL_Rect winnerHintRect{ 0, 0, 0, 0 };

    EngineSound engineSound;
    engineSound.init(); // if this fails, the app still runs (silently)
    const float maxCarSpeed = 130.0f;
    const float playerAccel = 60.0f;
    const float playerBrake = 90.0f;
    const float playerSteerRate = 70.0f;
    const float laneLimit = trackWidth / 2.0f - 12.0f;
    const float aiAccel = 50.0f; // how fast AI cars regain their cruising speed after a collision
    const float kRecoveryBoost = 2.5f; // extra acceleration multiplier right after a crash

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
                } else if (k == SDLK_2 && player2Index < 0 && !raceFinished) {
                    // Add player 2: pick a random AI car.
                    int newP2 = Car::assignPlayer2(cars, window, player1Index);
                    if (newP2 >= 0) {
                        player2Index = newP2;
                        player2LabelTexture = makeLabelTexture(renderer, font, "Player 2",
                                                                 cars[player2Index].getColor(), player2LabelRect);
                        player2LabelRect.y = player1LabelRect.y + player1LabelRect.h + 14;
                        player2LastLaps = -1; // force lap HUD refresh next frame
                    }
                } else if (k == SDLK_1 && player2Index >= 0 && !raceFinished) {
                    // Remove player 2: hand the car back to the AI.
                    Car::removePlayer2(cars, window, player1Index, player2Index);
                    player2Index = -1;
                    if (player2LabelTexture) {
                        SDL_DestroyTexture(player2LabelTexture);
                        player2LabelTexture = nullptr;
                    }
                    if (player2LapTexture) {
                        SDL_DestroyTexture(player2LapTexture);
                        player2LapTexture = nullptr;
                    }
                    player2LastLaps = -1;
                }
            }
        }

        Uint64 nowTicks = SDL_GetTicks64();
        float dt = static_cast<float>(nowTicks - lastTicks) / 1000.0f;
        lastTicks = nowTicks;
        if (dt > 0.05f) dt = 0.05f; // avoid large jumps (e.g. after a pause/resize)

        const Uint8* keys = SDL_GetKeyboardState(nullptr);

        const float totalLength = track.totalLength();
        if (!raceFinished) {
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

        if (cars[player1Index].laps != player1LastLaps) {
            if (player1LapTexture) SDL_DestroyTexture(player1LapTexture);
            std::string text = "Lap " + std::to_string(cars[player1Index].laps);
            player1LapTexture = makeLabelTexture(renderer, font, text.c_str(), cars[player1Index].getColor(), player1LapRect);
            player1LapRect.x = windowWidth - 20 - player1LapRect.w;
            player1LastLaps = cars[player1Index].laps;
        }
        if (player2Index >= 0 && cars[player2Index].laps != player2LastLaps) {
            if (player2LapTexture) SDL_DestroyTexture(player2LapTexture);
            std::string text = "Lap " + std::to_string(cars[player2Index].laps);
            player2LapTexture = makeLabelTexture(renderer, font, text.c_str(), cars[player2Index].getColor(), player2LapRect);
            player2LapRect.x = windowWidth - 20 - player2LapRect.w;
            player2LapRect.y = player1LapRect.y + player1LapRect.h + 14;
            player2LastLaps = cars[player2Index].laps;
        }

        // Grass background.
        SDL_SetRenderDrawColor(renderer, 34, 120, 50, 255);
        SDL_RenderClear(renderer);

        track.render(renderer);

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
        if (player1LapTexture) {
            SDL_Rect background{
                player1LapRect.x - 8, player1LapRect.y - 6,
                player1LapRect.w + 16, player1LapRect.h + 12
            };
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
            SDL_RenderFillRect(renderer, &background);
            SDL_RenderCopy(renderer, player1LapTexture, nullptr, &player1LapRect);
        }
        if (player2LapTexture) {
            SDL_Rect background{
                player2LapRect.x - 8, player2LapRect.y - 6,
                player2LapRect.w + 16, player2LapRect.h + 12
            };
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
            SDL_RenderFillRect(renderer, &background);
            SDL_RenderCopy(renderer, player2LapTexture, nullptr, &player2LapRect);
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
    if (player1LapTexture) SDL_DestroyTexture(player1LapTexture);
    if (player2LapTexture) SDL_DestroyTexture(player2LapTexture);
    if (winnerTexture) SDL_DestroyTexture(winnerTexture);
    if (winnerHintTexture) SDL_DestroyTexture(winnerHintTexture);
    engineSound.shutdown();
    SDL_DestroyTexture(carTexture);
    shutdownApp(app);

    return 0;
}
