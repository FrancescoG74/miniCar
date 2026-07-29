#include "actor/Car.h"

#include <algorithm>
#include <cmath>
#include "Track.h"
#include "input/AiInput.h"
#include "input/InputController.h"
#include "input/KeyboardInput.h"

Car::Car() : m_input(std::make_unique<AiInput>()) {}

Car::Car(float s_, float laneOffset_, float speed_, SDL_Color color_,
          const char* name_, float targetSpeed_)
    : Actor(name_, color_),
       s(s_), laneOffset(laneOffset_), targetLaneOffset(laneOffset_),
       speed(speed_), targetSpeed(targetSpeed_),
    hasStarted(s_ >= 0.0f),
    m_input(std::make_unique<AiInput>()) {}

Car::~Car() = default;
Car::Car(Car&&) noexcept = default;
Car& Car::operator=(Car&&) noexcept = default;

void Car::update(float dt, const std::vector<Car>& allCars, size_t selfIndex,
                  float totalLength, const CarControls& ctrl) {
    if (m_input) {
        m_input->drive(*this, dt, allCars, selfIndex, totalLength, ctrl);
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

void Car::setDriver(DriverKind driver) {
    m_driver = driver;
    switch (m_driver) {
    case DriverKind::Player1:
        m_input = std::make_unique<KeyboardInput>(KeyboardInput::wasd());
        break;
    case DriverKind::Player2:
        m_input = std::make_unique<KeyboardInput>(KeyboardInput::ijkl());
        break;
    case DriverKind::Ai:
        m_input = std::make_unique<AiInput>();
        // Let AiInput choose its configured home lane on the next update.
        targetLaneOffset = 0.0f;
        break;
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
