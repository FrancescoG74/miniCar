#include "CollisionSystem.h"

#include <cmath>

#include "TrackMath.h"
#include "actor/Car.h"
#include "actor/Gate.h"
#include "actor/Rock.h"

using track_math::wrapS;
using track_math::wrappedGap;

void CollisionSystem::resolveAll(std::vector<Car>& cars,
                                 const std::vector<Rock>& rocks,
                                 const std::vector<Gate>& gates,
                                 const Config& cfg) {
    carVsCar(cars, cfg);
    carVsRock(cars, rocks, cfg);
    carVsGate(cars, gates, cfg);
}

void CollisionSystem::carVsCar(std::vector<Car>& cars, const Config& cfg) {
    for (size_t i = 0; i < cars.size(); ++i) {
        for (size_t j = i + 1; j < cars.size(); ++j) {
            Car& a = cars[i];
            Car& b = cars[j];

            // Wrap-aware gap along the track to figure out who is ahead of whom.
            const float gap = wrappedGap(b.s, a.s, cfg.totalLength);
            const float lateralGap = b.laneOffset - a.laneOffset;

            if (std::abs(gap) >= cfg.carLength || std::abs(lateralGap) >= cfg.carWidth) continue;

            const float push = (cfg.carLength - std::abs(gap)) / 2.0f + 1.0f;
            if (gap >= 0.0f) {
                b.s = wrapS(b.s + push, cfg.totalLength);
                a.s = wrapS(a.s - push, cfg.totalLength);
            } else {
                a.s = wrapS(a.s + push, cfg.totalLength);
                b.s = wrapS(b.s - push, cfg.totalLength);
            }

            a.speed = 0.0f;
            b.speed = 0.0f;
            a.recoveryTimer = Car::kRecoveryDuration;
            b.recoveryTimer = Car::kRecoveryDuration;
        }
    }
}

void CollisionSystem::carVsRock(std::vector<Car>& cars,
                                const std::vector<Rock>& rocks,
                                const Config& cfg) {
    for (auto& car : cars) {
        for (const auto& rock : rocks) {
            if (!rock.active) continue;

            const float gap = wrappedGap(rock.s, car.s, cfg.totalLength);
            const float lateralGap = rock.laneOffset - car.laneOffset;

            // Sum of half-extents: car footprint + rock's *collision* radius. Because
            // the rock renders as an irregular octagon (not a full square of side
            // 2*size), we use ~55% of the drawing size as the collision radius. This
            // matches the visible polygon more closely and, combined with placing
            // rocks at |laneOffset| >= 45, keeps the hitboxes clear of the AI's ±25
            // lanes so AI cars never oscillate against a rock they can't avoid.
            const float rockCollisionRadius = rock.size * 0.55f;
            const float sExtent = cfg.carLength / 2.0f + rockCollisionRadius;
            const float latExtent = cfg.carWidth / 2.0f + rockCollisionRadius;

            if (std::abs(gap) >= sExtent || std::abs(lateralGap) >= latExtent) continue;

            // Push the car back along the track so it isn't overlapping the rock any
            // more (the rock is static, only the car moves).
            const float push = sExtent - std::abs(gap) + 1.0f;
            if (gap >= 0.0f) {
                car.s = wrapS(car.s - push, cfg.totalLength);
            } else {
                car.s = wrapS(car.s + push, cfg.totalLength);
            }

            car.speed = 0.0f;
            car.recoveryTimer = Car::kRecoveryDuration;
        }
    }
}

void CollisionSystem::carVsGate(std::vector<Car>& cars,
                                const std::vector<Gate>& gates,
                                const Config& cfg) {
    for (auto& car : cars) {
        for (const auto& gate : gates) {
            if (!gate.active || !gate.isClosed()) continue;

            const float gap = wrappedGap(gate.s, car.s, cfg.totalLength);
            const float sExtent = cfg.carLength / 2.0f + gate.thickness / 2.0f;
            if (std::abs(gap) >= sExtent) continue;

            // Gates span the full track width, so any car in the s-overlap collides.
            const float push = sExtent - std::abs(gap) + 1.0f;
            if (gap >= 0.0f) {
                car.s = wrapS(car.s - push, cfg.totalLength);
            } else {
                car.s = wrapS(car.s + push, cfg.totalLength);
            }
            car.speed = 0.0f;
            car.recoveryTimer = Car::kRecoveryDuration;
        }
    }
}
