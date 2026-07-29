#include "input/KeyboardInput.h"

#include <SDL2/SDL.h>
#include <algorithm>

#include "actor/Car.h"

KeyboardInput::Scheme KeyboardInput::wasd() {
    return { SDL_SCANCODE_W, SDL_SCANCODE_S, SDL_SCANCODE_A, SDL_SCANCODE_D };
}

KeyboardInput::Scheme KeyboardInput::ijkl() {
    return { SDL_SCANCODE_I, SDL_SCANCODE_K, SDL_SCANCODE_J, SDL_SCANCODE_L };
}

void KeyboardInput::drive(Car& car, float dt,
                          const std::vector<Car>& /*allCars*/, std::size_t /*selfIndex*/,
                          float /*totalLength*/, const CarControls& ctrl) {
    if (!ctrl.keys) return;
    if (ctrl.keys[m_scheme.accelerate])
        car.speed = std::min(ctrl.maxSpeed, car.speed + ctrl.playerAccel * dt);
    if (ctrl.keys[m_scheme.brake])
        car.speed = std::max(0.0f, car.speed - ctrl.playerBrake * dt);
    if (ctrl.keys[m_scheme.left])
        car.laneOffset = std::max(-ctrl.laneLimit, car.laneOffset - ctrl.playerSteerRate * dt);
    if (ctrl.keys[m_scheme.right])
        car.laneOffset = std::min(ctrl.laneLimit, car.laneOffset + ctrl.playerSteerRate * dt);
}
