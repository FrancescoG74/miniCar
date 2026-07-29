#pragma once

#include <SDL2/SDL.h>
#include <vector>

#include "actor/Actor.h"

class Track;

// Bundles the tunable physics/input parameters shared by every car so the per-frame
// Car::update() call site stays compact.
struct CarControls {
    const Uint8* keys = nullptr;
    float maxSpeed = 130.0f;
    float playerAccel = 60.0f;
    float playerBrake = 90.0f;
    float playerSteerRate = 70.0f;
    float laneLimit = 0.0f;
    float aiAccel = 50.0f;
    float recoveryBoost = 2.5f;

    // Optional list of track-arc-length positions that AI cars must slow down for
    // (e.g. currently-closed gates). Decoupled from Gate so Car doesn't need to
    // know about it -- the game loop rebuilds this list every frame.
    const std::vector<float>* blockedSPositions = nullptr;

    // Track used to refresh Actor::position (world-space) each frame. Optional;
    // if null, Car::update leaves position unchanged.
    const Track* track = nullptr;
};

class Car : public Actor {
public:
    // Seconds an AI car receives a post-crash acceleration boost.
    static constexpr float kRecoveryDuration = 1.5f;

    // Public data: the game loop and HUD read many of these fields directly.
    // Identity fields (name, color, id, active) live on the Actor base class.
    float s = 0.0f;              // distance traveled along the track centerline
    float laneOffset = 0.0f;     // perpendicular offset from centerline (lane position)
    float targetLaneOffset = 0.0f; // AI-only: lane the car is currently steering toward
    float speed = 0.0f;          // pixels per second
    int playerNumber = 0;        // 0 = AI, 1 = player 1 (WASD), 2 = player 2 (IJKL)
    float distanceTraveled = 0.0f; // cumulative distance since race start (never wraps)
    int laps = 0;
    float targetSpeed = 0.0f;    // cruising speed AI cars accelerate back to after a collision
    float recoveryTimer = 0.0f;  // while > 0, AI accelerates faster to recover from a crash

    Car() = default;
    Car(float s, float laneOffset, float speed, SDL_Color color,
         const char* name, float targetSpeed);

    // Advances this car by one frame: applies keyboard input (player cars) or the
    // adaptive-cruise AI (AI cars), then updates s, distanceTraveled and laps.
    // `allCars`/`selfIndex` let the AI look at other cars in its own lane so it
    // follows a slower car ahead instead of ramming it.
    void update(float dt, const std::vector<Car>& allCars, size_t selfIndex,
                 float totalLength, const CarControls& ctrl);

    // -- Static helpers for the whole race --------------------------------------

    // Builds the fixed race grid (10 cars, five staggered rows of two lanes).
    static std::vector<Car> createInitialGrid(float laneOffset);

    // Picks a random car, marks it as player 1, prints the assignment and (if
    // non-null) refreshes the window title. Returns the chosen index.
    static int assignPlayer1(std::vector<Car>& cars, SDL_Window* window);

    // Picks a random AI car (not equal to player1Index), marks it as player 2 and
    // refreshes the window title. Returns the new player 2 index, or -1 if no AI
    // car is available.
    static int assignPlayer2(std::vector<Car>& cars, SDL_Window* window,
                                int player1Index);

    // Turns the car at `player2Index` back into an AI car and refreshes the window
    // title to show only player 1.
    static void removePlayer2(std::vector<Car>& cars, SDL_Window* window,
                                int player1Index, int player2Index);

    // Creates a simple white "car" sprite texture the caller can tint per-car and
    // must destroy with SDL_DestroyTexture.
    static SDL_Texture* createTexture(SDL_Renderer* renderer, int width, int height);

    // Detects cars that are too close and, when found, zeroes both cars' speed and
    // nudges them apart. The collision area is a rectangle matching the car's
    // footprint (`carLength` along direction of travel, `carWidthDim` across lanes).
    static void resolveCollisions(std::vector<Car>& cars, float totalLength,
                                   float carLength, float carWidthDim);
};
