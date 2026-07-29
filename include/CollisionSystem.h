#pragma once

#include <vector>

class Car;
class Rock;
class Gate;

// Owns all per-frame collision resolution. Replaces the trio of static
// X::resolveCarCollisions() helpers that used to live on Car / Rock / Gate and
// each re-implemented the same wrap-aware track math.
//
// Every resolver treats cars as movable, rocks/gates as static, and reuses the
// same convention: on collision the car's speed is zeroed, its recovery boost
// is armed, and it is pushed back along the track just far enough to clear.
class CollisionSystem {
public:
    struct Config {
        float totalLength = 0.0f;  // circuit length (arc-length domain)
        float carLength = 0.0f;    // car footprint along the direction of travel
        float carWidth = 0.0f;     // car footprint across lanes
    };

    // Convenience: run all three passes in the same order the game loop used to.
    static void resolveAll(std::vector<Car>& cars,
                           const std::vector<Rock>& rocks,
                           const std::vector<Gate>& gates,
                           const Config& cfg);

    static void carVsCar(std::vector<Car>& cars, const Config& cfg);
    static void carVsRock(std::vector<Car>& cars,
                          const std::vector<Rock>& rocks,
                          const Config& cfg);
    static void carVsGate(std::vector<Car>& cars,
                          const std::vector<Gate>& gates,
                          const Config& cfg);
};
