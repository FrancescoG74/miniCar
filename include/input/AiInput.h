#pragma once

#include "input/InputController.h"

// Adaptive-cruise racing AI: home into one of two racing lanes (kHomeLaneMagnitude
// on either side of the centerline), scan for a slower car ahead in the current
// lane, try to switch to the opposite lane when safe, cap speed to the car directly
// ahead in the same lane, and slow down before any blocked track positions
// (currently-closed gates). The logic is a straight port of what used to live
// inside Car::update() -- extracted so each strategy stays small and swappable.
class AiInput : public InputController {
public:
    void drive(Car& car, float dt,
               const std::vector<Car>& allCars, std::size_t selfIndex,
               float totalLength, const CarControls& ctrl) override;
};
