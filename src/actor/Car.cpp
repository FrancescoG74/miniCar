#include "actor/Car.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <random>
#include <string>

#include "Track.h"
#include "input/AiInput.h"
#include "input/InputController.h"
#include "input/KeyboardInput.h"

Car::Car() : input(std::make_unique<AiInput>()) {}

Car::Car(float s_, float laneOffset_, float speed_, SDL_Color color_,
          const char* name_, float targetSpeed_)
    : Actor(name_, color_),
       s(s_), laneOffset(laneOffset_), targetLaneOffset(laneOffset_),
       speed(speed_), targetSpeed(targetSpeed_),
       hasStarted(s_ >= 0.0f),
       input(std::make_unique<AiInput>()) {}

Car::~Car() = default;
Car::Car(Car&&) noexcept = default;
Car& Car::operator=(Car&&) noexcept = default;

void Car::update(float dt, const std::vector<Car>& allCars, size_t selfIndex,
                  float totalLength, const CarControls& ctrl) {
    if (input) {
        input->drive(*this, dt, allCars, selfIndex, totalLength, ctrl);
    }

    float newS = s + speed * dt;
    distanceTraveled += speed * dt;

    // Only credit a lap once the car's arc-length position actually wraps past
    // the start/finish line (s == 0), not just after accumulating one lap's
    // worth of raw distance. Cars spawn staggered behind the line (negative s),
    // so the very first crossing merely brings them up to the line -- it's the
    // race start, not a completed lap -- and is skipped via hasStarted.
    if (newS >= totalLength) {
        int crossings = static_cast<int>(std::floor(newS / totalLength));
        if (!hasStarted) {
            hasStarted = true;
            --crossings;
        }
        if (crossings > 0) laps += crossings;
    }
    s = std::fmod(newS, totalLength);
    if (s < 0.0f) s += totalLength;

    // Refresh the world-space position (Actor::position) so renderers and other
    // systems can read it directly without re-sampling the track.
    if (ctrl.track) {
        TrackPoint p = ctrl.track->sample(s);
        float perpX = -std::sin(p.angle);
        float perpY = std::cos(p.angle);
        setPosition({ p.x + perpX * laneOffset, p.y + perpY * laneOffset });
    }
}

std::vector<Car> Car::createInitialGrid(float laneOffset) {
    // All rows are staggered strictly behind the start line (s < 0) so every car
    // must actually drive up to and cross the line before lap 1 begins -- none of
    // them start sitting exactly on top of it. Built with emplace_back because
    // Car is move-only (owns its InputController via unique_ptr).
    std::vector<Car> grid;
    grid.reserve(6);
    grid.emplace_back(-45.0f,  -laneOffset, 90.0f,  SDL_Color{ 220, 40, 40, 255 },  "Red",    90.0f);
    grid.emplace_back(-45.0f,   laneOffset, 100.0f, SDL_Color{ 40, 90, 220, 255 },  "Blue",   100.0f);
    grid.emplace_back(-90.0f,  -laneOffset, 80.0f,  SDL_Color{ 40, 180, 60, 255 },  "Green",  80.0f);
    grid.emplace_back(-90.0f,   laneOffset, 110.0f, SDL_Color{ 230, 200, 30, 255 }, "Yellow", 110.0f);
    grid.emplace_back(-135.0f, -laneOffset, 95.0f,  SDL_Color{ 230, 120, 30, 255 }, "Orange", 95.0f);
    grid.emplace_back(-135.0f,  laneOffset, 105.0f, SDL_Color{ 150, 60, 200, 255 }, "Purple", 105.0f);
    return grid;
}

namespace {

void updateWindowTitle(const std::vector<Car>& cars, SDL_Window* window,
                        int player1Index, int player2Index) {
    if (!window) return;
    std::string title = "miniCar - P1: ";
    title += cars[player1Index].getName();
    title += " (WASD)";
    if (player2Index >= 0) {
        title += "  |  P2: ";
        title += cars[player2Index].getName();
        title += " (IJKL)";
    } else {
        title += "  |  press 2 to add P2";
    }
    SDL_SetWindowTitle(window, title.c_str());
}

} // namespace

int Car::assignPlayer1(std::vector<Car>& cars, SDL_Window* window) {
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<int> dist(0, static_cast<int>(cars.size()) - 1);
    int player1Index = dist(rng);
    cars[player1Index].playerNumber = 1;
    cars[player1Index].input = std::make_unique<KeyboardInput>(KeyboardInput::wasd());

    std::cout << "Player 1 controls the " << cars[player1Index].getName() << " car (W/A/S/D).\n"
               << "Press '2' to add Player 2, '1' to remove them." << std::endl;
    updateWindowTitle(cars, window, player1Index, -1);
    return player1Index;
}

int Car::assignPlayer2(std::vector<Car>& cars, SDL_Window* window, int player1Index) {
    // Collect currently-AI cars (skip whichever is Player 1) and pick one at random.
    std::vector<int> candidates;
    for (size_t i = 0; i < cars.size(); ++i) {
        if (static_cast<int>(i) == player1Index) continue;
        if (cars[i].playerNumber == 0) candidates.push_back(static_cast<int>(i));
    }
    if (candidates.empty()) return -1;

    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<int> dist(0, static_cast<int>(candidates.size()) - 1);
    int player2Index = candidates[dist(rng)];
    cars[player2Index].playerNumber = 2;
    cars[player2Index].input = std::make_unique<KeyboardInput>(KeyboardInput::ijkl());

    std::cout << "Player 2 joined: controls the " << cars[player2Index].getName()
               << " car (I/J/K/L)." << std::endl;
    updateWindowTitle(cars, window, player1Index, player2Index);
    return player2Index;
}

void Car::removePlayer2(std::vector<Car>& cars, SDL_Window* window,
                          int player1Index, int player2Index) {
    if (player2Index < 0 || player2Index >= static_cast<int>(cars.size())) return;
    cars[player2Index].playerNumber = 0;
    cars[player2Index].input = std::make_unique<AiInput>();
    // Snap the AI's steering target to the nearest racing lane so it doesn't
    // wander off in whatever direction the player was pointing.
    cars[player2Index].targetLaneOffset =
        (cars[player2Index].laneOffset >= 0.0f) ? 25.0f : -25.0f;
    std::cout << "Player 2 left; " << cars[player2Index].getName()
               << " car returns to AI control." << std::endl;
    updateWindowTitle(cars, window, player1Index, -1);
}

SDL_TexturePtr Car::createTexture(SDL_Renderer* renderer, int width, int height) {
    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_RGBA32);
    SDL_FillRect(surface, nullptr, SDL_MapRGBA(surface->format, 255, 255, 255, 255));

    // Darker "windshield" near one end so rotation/orientation is visible.
    SDL_Rect windshield{ width / 4, height / 8, width / 2, height / 4 };
    SDL_FillRect(surface, &windshield, SDL_MapRGBA(surface->format, 40, 40, 40, 255));

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    SDL_FreeSurface(surface);
    return SDL_TexturePtr(texture);
}
