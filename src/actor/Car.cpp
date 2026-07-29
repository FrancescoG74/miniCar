#include "actor/Car.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <random>
#include <string>

#include "Track.h"

Car::Car(float s_, float laneOffset_, float speed_, SDL_Color color_,
          const char* name_, float targetSpeed_)
    : Actor(name_, color_),
       s(s_), laneOffset(laneOffset_), targetLaneOffset(laneOffset_),
       speed(speed_), targetSpeed(targetSpeed_) {}

void Car::update(float dt, const std::vector<Car>& allCars, size_t selfIndex,
                  float totalLength, const CarControls& ctrl) {
    if (playerNumber == 1) {
        if (ctrl.keys[SDL_SCANCODE_W]) speed = std::min(ctrl.maxSpeed, speed + ctrl.playerAccel * dt);
        if (ctrl.keys[SDL_SCANCODE_S]) speed = std::max(0.0f, speed - ctrl.playerBrake * dt);
        if (ctrl.keys[SDL_SCANCODE_A]) laneOffset = std::max(-ctrl.laneLimit, laneOffset - ctrl.playerSteerRate * dt);
        if (ctrl.keys[SDL_SCANCODE_D]) laneOffset = std::min(ctrl.laneLimit, laneOffset + ctrl.playerSteerRate * dt);
    } else if (playerNumber == 2) {
        if (ctrl.keys[SDL_SCANCODE_I]) speed = std::min(ctrl.maxSpeed, speed + ctrl.playerAccel * dt);
        if (ctrl.keys[SDL_SCANCODE_K]) speed = std::max(0.0f, speed - ctrl.playerBrake * dt);
        if (ctrl.keys[SDL_SCANCODE_J]) laneOffset = std::max(-ctrl.laneLimit, laneOffset - ctrl.playerSteerRate * dt);
        if (ctrl.keys[SDL_SCANCODE_L]) laneOffset = std::min(ctrl.laneLimit, laneOffset + ctrl.playerSteerRate * dt);
    } else {
        // -- Overtaking logic ---------------------------------------------------
        // AI cars home into one of two racing lanes (±kHomeLaneMagnitude). When a
        // slower car sits ahead in the current target lane, we try to switch to
        // the opposite lane so we can drive past instead of just tailgating.
        constexpr float kHomeLaneMagnitude = 25.0f;
        constexpr float kLaneMatchTolerance = 15.0f;  // laneOffset delta counted as "same lane"
        constexpr float kOvertakeLookAhead = 90.0f;   // start looking to overtake within this range
        constexpr float kOvertakeSpeedMargin = 5.0f;  // require the car ahead to be this much slower
        constexpr float kSafeGapFront = 100.0f;       // needed clear space ahead in the target lane
        constexpr float kSafeGapBack = 45.0f;         // needed clear space behind in the target lane
        constexpr float kAiSteerRate = 45.0f;         // lane-change rate in units/second

        // Make sure the target lane is one of the two racing lanes. If a human
        // just handed the car back to the AI it may be at an arbitrary offset.
        if (std::abs(targetLaneOffset) < 1.0f) {
            targetLaneOffset = (laneOffset >= 0.0f) ? kHomeLaneMagnitude : -kHomeLaneMagnitude;
        }

        // Is there a slower car ahead of me in the lane I'm currently heading to?
        bool blockedAhead = false;
        for (size_t j = 0; j < allCars.size(); ++j) {
            if (j == selfIndex) continue;
            const Car& other = allCars[j];
            if (std::abs(other.laneOffset - targetLaneOffset) > kLaneMatchTolerance) continue;

            float gap = other.s - s;
            while (gap > totalLength / 2.0f) gap -= totalLength;
            while (gap < -totalLength / 2.0f) gap += totalLength;

            if (gap > 0.0f && gap < kOvertakeLookAhead &&
                other.speed + kOvertakeSpeedMargin < targetSpeed) {
                blockedAhead = true;
                break;
            }
        }

        if (blockedAhead) {
            float altLane = -targetLaneOffset;
            bool altSafe = true;
            for (size_t j = 0; j < allCars.size(); ++j) {
                if (j == selfIndex) continue;
                const Car& other = allCars[j];
                if (std::abs(other.laneOffset - altLane) > kLaneMatchTolerance) continue;

                float gap = other.s - s;
                while (gap > totalLength / 2.0f) gap -= totalLength;
                while (gap < -totalLength / 2.0f) gap += totalLength;

                if (gap >= -kSafeGapBack && gap <= kSafeGapFront) {
                    altSafe = false;
                    break;
                }
            }
            if (altSafe) targetLaneOffset = altLane;
        }

        // Steer laneOffset toward the current target at a bounded rate so lane
        // changes look smooth rather than teleporting.
        float laneDelta = targetLaneOffset - laneOffset;
        float laneStep = kAiSteerRate * dt;
        if (laneDelta > laneStep) laneOffset += laneStep;
        else if (laneDelta < -laneStep) laneOffset -= laneStep;
        else laneOffset = targetLaneOffset;

        // AI cars can't change lanes, so a faster AI stuck behind a slower one in
        // the same lane would otherwise ram it every lap. Cap its speed to whatever's
        // directly ahead in the same lane so it follows instead of colliding.
        float aheadCarCap = ctrl.maxSpeed;
        for (size_t j = 0; j < allCars.size(); ++j) {
            if (j == selfIndex) continue;
            const Car& other = allCars[j];
            if (std::abs(other.laneOffset - laneOffset) > 35.0f) continue; // different lane

            float gap = other.s - s;
            while (gap > totalLength / 2.0f) gap -= totalLength;
            while (gap < -totalLength / 2.0f) gap += totalLength;

            if (gap > 0.0f && gap < 60.0f) {
                aheadCarCap = std::min(aheadCarCap, other.speed);
            }
        }

        // Slow down when approaching a currently-blocked spot (e.g. a closed gate).
        // The cap ramps down linearly with distance so the AI eases to a stop just
        // before the obstacle instead of oscillating against it. Kept separate from
        // the car-follow cap so post-crash recovery can ignore the follow cap while
        // still respecting static obstacles ahead.
        float obstacleCap = ctrl.maxSpeed;
        if (ctrl.blockedSPositions) {
            constexpr float kLookAhead = 120.0f;
            for (float blockedS : *ctrl.blockedSPositions) {
                float gap = blockedS - s;
                while (gap > totalLength / 2.0f) gap -= totalLength;
                while (gap < -totalLength / 2.0f) gap += totalLength;
                if (gap > 0.0f && gap < kLookAhead) {
                    float allowed = (gap / kLookAhead) * ctrl.maxSpeed;
                    obstacleCap = std::min(obstacleCap, allowed);
                }
            }
        }

        float desiredSpeed = std::min({ targetSpeed, aheadCarCap, obstacleCap });
        float accelRate = ctrl.aiAccel;
        if (recoveryTimer > 0.0f) {
            accelRate *= ctrl.recoveryBoost;
            recoveryTimer = std::max(0.0f, recoveryTimer - dt);
            // While recovering from a crash, ignore the follow-distance speed cap
            // (a chain-stopped car ahead would otherwise clamp us to 0), but keep
            // the obstacle cap so we still ease to a stop before a closed gate.
            desiredSpeed = std::min(targetSpeed, obstacleCap);
        }
        if (speed < desiredSpeed) {
            speed = std::min(desiredSpeed, speed + accelRate * dt);
        } else if (speed > desiredSpeed) {
            speed = std::max(desiredSpeed, speed - accelRate * dt);
        }
    }

    float newS = s + speed * dt;
    distanceTraveled += speed * dt;

    // Only credit a lap once the car's arc-length position actually wraps past
    // the start/finish line (s == 0), not just after accumulating one lap's
    // worth of raw distance. Cars spawn staggered slightly behind the line
    // (negative s), so counting via distanceTraveled/totalLength would credit
    // a lap ~before they've physically crossed it.
    if (newS >= totalLength) {
        laps += static_cast<int>(std::floor(newS / totalLength));
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

std::vector<Car> Car::createInitialGrid(float laneOffset) {
    return {
        Car( 0.0f,   -laneOffset, 90.0f,  SDL_Color{ 220, 40, 40, 255 },  "Red",    90.0f),
        Car( 0.0f,    laneOffset, 100.0f, SDL_Color{ 40, 90, 220, 255 },  "Blue",   100.0f),
        Car(-45.0f,  -laneOffset, 80.0f,  SDL_Color{ 40, 180, 60, 255 },  "Green",  80.0f),
        Car(-45.0f,   laneOffset, 110.0f, SDL_Color{ 230, 200, 30, 255 }, "Yellow", 110.0f),
        Car(-90.0f,  -laneOffset, 95.0f,  SDL_Color{ 230, 120, 30, 255 }, "Orange", 95.0f),
        Car(-90.0f,   laneOffset, 105.0f, SDL_Color{ 150, 60, 200, 255 }, "Purple", 105.0f),
    };
}

namespace {

void updateWindowTitle(const std::vector<Car>& cars, SDL_Window* window,
                        int player1Index, int player2Index) {
    if (!window) return;
    std::string title = "miniCar - P1: ";
    title += cars[player1Index].getName();
    title += " (WASD)";
    if (player2Index >= 0) {
        title += "  |  P2: ";
        title += cars[player2Index].getName();
        title += " (IJKL)";
    } else {
        title += "  |  press 2 to add P2";
    }
    SDL_SetWindowTitle(window, title.c_str());
}

} // namespace

int Car::assignPlayer1(std::vector<Car>& cars, SDL_Window* window) {
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<int> dist(0, static_cast<int>(cars.size()) - 1);
    int player1Index = dist(rng);
    cars[player1Index].playerNumber = 1;

    std::cout << "Player 1 controls the " << cars[player1Index].getName() << " car (W/A/S/D).\n"
               << "Press '2' to add Player 2, '1' to remove them." << std::endl;
    updateWindowTitle(cars, window, player1Index, -1);
    return player1Index;
}

int Car::assignPlayer2(std::vector<Car>& cars, SDL_Window* window, int player1Index) {
    // Collect currently-AI cars (skip whichever is Player 1) and pick one at random.
    std::vector<int> candidates;
    for (size_t i = 0; i < cars.size(); ++i) {
        if (static_cast<int>(i) == player1Index) continue;
        if (cars[i].playerNumber == 0) candidates.push_back(static_cast<int>(i));
    }
    if (candidates.empty()) return -1;

    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<int> dist(0, static_cast<int>(candidates.size()) - 1);
    int player2Index = candidates[dist(rng)];
    cars[player2Index].playerNumber = 2;

    std::cout << "Player 2 joined: controls the " << cars[player2Index].getName()
               << " car (I/J/K/L)." << std::endl;
    updateWindowTitle(cars, window, player1Index, player2Index);
    return player2Index;
}

void Car::removePlayer2(std::vector<Car>& cars, SDL_Window* window,
                          int player1Index, int player2Index) {
    if (player2Index < 0 || player2Index >= static_cast<int>(cars.size())) return;
    cars[player2Index].playerNumber = 0;
    // Snap the AI's steering target to the nearest racing lane so it doesn't
    // wander off in whatever direction the player was pointing.
    cars[player2Index].targetLaneOffset =
        (cars[player2Index].laneOffset >= 0.0f) ? 25.0f : -25.0f;
    std::cout << "Player 2 left; " << cars[player2Index].getName()
               << " car returns to AI control." << std::endl;
    updateWindowTitle(cars, window, player1Index, -1);
}

SDL_Texture* Car::createTexture(SDL_Renderer* renderer, int width, int height) {
    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_RGBA32);
    SDL_FillRect(surface, nullptr, SDL_MapRGBA(surface->format, 255, 255, 255, 255));

    // Darker "windshield" near one end so rotation/orientation is visible.
    SDL_Rect windshield{ width / 4, height / 8, width / 2, height / 4 };
    SDL_FillRect(surface, &windshield, SDL_MapRGBA(surface->format, 40, 40, 40, 255));

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    SDL_FreeSurface(surface);
    return texture;
}

void Car::resolveCollisions(std::vector<Car>& cars, float totalLength,
                              float carLength, float carWidthDim) {
    for (size_t i = 0; i < cars.size(); ++i) {
        for (size_t j = i + 1; j < cars.size(); ++j) {
            Car& a = cars[i];
            Car& b = cars[j];

            // Wrap-aware gap along the track to figure out who is ahead of whom.
            float gap = b.s - a.s;
            while (gap > totalLength / 2.0f) gap -= totalLength;
            while (gap < -totalLength / 2.0f) gap += totalLength;
            float lateralGap = b.laneOffset - a.laneOffset;

            if (std::abs(gap) >= carLength || std::abs(lateralGap) >= carWidthDim) continue;

            float push = (carLength - std::abs(gap)) / 2.0f + 1.0f;
            if (gap >= 0.0f) {
                b.s = std::fmod(b.s + push + totalLength, totalLength);
                a.s = std::fmod(a.s - push + totalLength, totalLength);
            } else {
                a.s = std::fmod(a.s + push + totalLength, totalLength);
                b.s = std::fmod(b.s - push + totalLength, totalLength);
            }

            a.speed = 0.0f;
            b.speed = 0.0f;
            a.recoveryTimer = kRecoveryDuration;
            b.recoveryTimer = kRecoveryDuration;
        }
    }
}
