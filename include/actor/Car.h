#pragma once

#include <SDL2/SDL.h>
#include <memory>
#include <vector>

#include "SdlRaii.h"
#include "actor/Actor.h"
#include "game/RaceConstants.h"

class InputController;
class Track;
struct AiTuning;

// The authoritative identity of the current driver. Car uses it to choose and
// own its InputController; callers must not manage controllers directly.
enum class DriverKind {
    Ai,
    Player1,
    Player2,
};

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

    // Immutable AI behavior values; only consumed by AiInput.
    const AiTuning* aiTuning = nullptr;
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
    float distanceTraveled = 0.0f; // cumulative distance since race start (never wraps)
    int laps = 0;
    float targetSpeed = 0.0f;    // cruising speed AI cars accelerate back to after a collision
    float recoveryTimer = 0.0f;  // while > 0, AI accelerates faster to recover from a crash
    // True once this car has passed the start line for the first time. Grid cars
    // spawn staggered behind s = 0, so their first crossing just brings them up to
    // the line (race start) and must not itself be counted as a completed lap.
    bool hasStarted = false;

    Car();
    Car(float s, float laneOffset, float speed, SDL_Color color,
         const char* name, float targetSpeed);
    ~Car();

    // Move-only: unique_ptr member forbids copying.
    Car(const Car&) = delete;
    Car& operator=(const Car&) = delete;
    Car(Car&&) noexcept;
    Car& operator=(Car&&) noexcept;

    // Advances this car by one frame: delegates input/AI to `input`, then updates
    // s, distanceTraveled and laps. `allCars`/`selfIndex` let the AI look at
    // other cars to overtake or follow instead of ramming them.
    void update(float dt, const std::vector<Car>& allCars, size_t selfIndex,
                 float totalLength, const CarControls& ctrl);

    void applyCollision();

    DriverKind driver() const { return m_driver; }
    void setDriver(DriverKind driver);

    // -- Static helpers for the whole race --------------------------------------

    // Builds the fixed race grid (three staggered rows of two lanes).
    static std::vector<Car> createInitialGrid(float laneOffset);

    // Creates a simple white "car" sprite texture the caller can tint per-car.
    // Returned as an owning SDL_TexturePtr so the caller doesn't have to
    // remember SDL_DestroyTexture.
    static SDL_TexturePtr createTexture(SDL_Renderer* renderer, int width, int height);

private:
    DriverKind m_driver = DriverKind::Ai;
    std::unique_ptr<InputController> m_input;
};
