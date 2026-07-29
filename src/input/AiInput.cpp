#include "input/AiInput.h"

#include <algorithm>
#include <cmath>

#include "TrackMath.h"
#include "actor/Car.h"
#include "game/RaceTuning.h"

using track_math::wrappedGap;

void AiInput::drive(Car& car, float dt,
                    const std::vector<Car>& allCars, std::size_t selfIndex,
                    float totalLength, const CarControls& ctrl) {
    // AI cars home into one of two racing lanes (±kHomeLaneMagnitude). When a
    // slower car sits ahead in the current target lane, we try to switch to the
    // opposite lane so we can drive past instead of just tailgating.
    const AiTuning defaults{};
    const AiTuning& tuning = ctrl.aiTuning ? *ctrl.aiTuning : defaults;

    // If a human just handed the car back to the AI it may be at an arbitrary
    // offset; snap the target back to one of the two racing lanes.
    if (std::abs(car.targetLaneOffset) < 1.0f) {
        car.targetLaneOffset = car.laneOffset >= 0.0f ? tuning.homeLaneOffset : -tuning.homeLaneOffset;
    }

    // Is there a slower car ahead of me in the lane I'm currently heading to?
    bool blockedAhead = false;
    for (std::size_t j = 0; j < allCars.size(); ++j) {
        if (j == selfIndex) continue;
        const Car& other = allCars[j];
        if (std::abs(other.laneOffset - car.targetLaneOffset) > tuning.laneMatchTolerance) continue;

        float gap = wrappedGap(other.s, car.s, totalLength);
        if (gap > 0.0f && gap < tuning.overtakeLookAhead &&
            other.speed + tuning.overtakeSpeedMargin < car.targetSpeed) {
            blockedAhead = true;
            break;
        }
    }

    if (blockedAhead) {
        float altLane = -car.targetLaneOffset;
        bool altSafe = true;
        for (std::size_t j = 0; j < allCars.size(); ++j) {
            if (j == selfIndex) continue;
            const Car& other = allCars[j];
            if (std::abs(other.laneOffset - altLane) > tuning.laneMatchTolerance) continue;

            float gap = wrappedGap(other.s, car.s, totalLength);
            if (gap >= -tuning.safeGapBack && gap <= tuning.safeGapFront) {
                altSafe = false;
                break;
            }
        }
        if (altSafe) car.targetLaneOffset = altLane;
    }

    // Steer laneOffset toward the current target at a bounded rate so lane
    // changes look smooth rather than teleporting.
    float laneDelta = car.targetLaneOffset - car.laneOffset;
    float laneStep = tuning.steerRate * dt;
    if (laneDelta > laneStep) car.laneOffset += laneStep;
    else if (laneDelta < -laneStep) car.laneOffset -= laneStep;
    else car.laneOffset = car.targetLaneOffset;

    // AI cars can't change lanes freely, so cap speed to whatever's directly
    // ahead in the same lane instead of just ramming it.
    float aheadCarCap = ctrl.maxSpeed;
    for (std::size_t j = 0; j < allCars.size(); ++j) {
        if (j == selfIndex) continue;
        const Car& other = allCars[j];
        if (std::abs(other.laneOffset - car.laneOffset) > tuning.followingLaneTolerance) continue;

        float gap = wrappedGap(other.s, car.s, totalLength);
        if (gap > 0.0f && gap < tuning.followingLookAhead) {
            aheadCarCap = std::min(aheadCarCap, other.speed);
        }
    }

    // Slow down when approaching a currently-blocked spot (e.g. a closed gate).
    // The cap ramps down linearly with distance so the AI eases to a stop just
    // before the obstacle instead of oscillating against it.
    float obstacleCap = ctrl.maxSpeed;
    if (ctrl.blockedSPositions) {
        for (float blockedS : *ctrl.blockedSPositions) {
            float gap = wrappedGap(blockedS, car.s, totalLength);
            if (gap > 0.0f && gap < tuning.obstacleLookAhead) {
                float allowed = (gap / tuning.obstacleLookAhead) * ctrl.maxSpeed;
                obstacleCap = std::min(obstacleCap, allowed);
            }
        }
    }

    float desiredSpeed = std::min({ car.targetSpeed, aheadCarCap, obstacleCap });
    float accelRate = ctrl.aiAccel;
    if (car.recoveryTimer > 0.0f) {
        accelRate *= ctrl.recoveryBoost;
        car.recoveryTimer = std::max(0.0f, car.recoveryTimer - dt);
        // While recovering from a crash, ignore the follow-distance speed cap
        // (a chain-stopped car ahead would otherwise clamp us to 0), but keep
        // the obstacle cap so we still ease to a stop before a closed gate.
        desiredSpeed = std::min(car.targetSpeed, obstacleCap);
    }
    if (car.speed < desiredSpeed) {
        car.speed = std::min(desiredSpeed, car.speed + accelRate * dt);
    } else if (car.speed > desiredSpeed) {
        car.speed = std::max(desiredSpeed, car.speed - accelRate * dt);
    }
}
