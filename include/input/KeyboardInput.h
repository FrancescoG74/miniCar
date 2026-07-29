#pragma once

#include <SDL2/SDL_scancode.h>

#include "input/InputController.h"

// Reads a keyboard layout (accelerate/brake/left/right scancodes) and applies
// the same accel/brake/steer logic that used to live inline in Car::update.
// Two layouts are pre-baked (WASD for Player 1, IJKL for Player 2) but the
// scheme is a plain struct so new bindings (arrow keys, gamepad-mapped
// scancodes, etc.) can be added without touching the class.
class KeyboardInput : public InputController {
public:
    struct Scheme {
        SDL_Scancode accelerate;
        SDL_Scancode brake;
        SDL_Scancode left;
        SDL_Scancode right;
    };

    static Scheme wasd();
    static Scheme ijkl();

    explicit KeyboardInput(Scheme scheme) : m_scheme(scheme) {}

    void drive(Car& car, float dt,
               const std::vector<Car>& allCars, std::size_t selfIndex,
               float totalLength, const CarControls& ctrl) override;

private:
    Scheme m_scheme;
};
