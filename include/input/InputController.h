#pragma once

#include <cstddef>
#include <vector>

class Car;
struct CarControls;

// Strategy interface for deciding how a Car's `speed` and `laneOffset` change
// each frame. Concrete strategies swap in per-car:
//   - `KeyboardInput` reads WASD / IJKL and drives directly.
//   - `AiInput` runs the adaptive-cruise / overtake logic.
//
// The strategy is only responsible for the pre-advance input step; the
// arc-length integration, lap counting and world-space position refresh stay
// on Car::update itself so those invariants can't drift per-controller.
class InputController {
public:
    virtual ~InputController() = default;

    virtual void drive(Car& car, float dt,
                       const std::vector<Car>& allCars, std::size_t selfIndex,
                       float totalLength, const CarControls& ctrl) = 0;
};
