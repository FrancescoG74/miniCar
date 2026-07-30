#include <catch2/catch_all.hpp>
#include <cmath>
#include <vector>

#include "CollisionSystem.h"
#include "Track.h"
#include "actor/Car.h"
#include "actor/Gate.h"
#include "actor/Rock.h"

namespace {
Car makeCar(float s, float laneOffset, float speed) {
    return Car(s, laneOffset, speed, SDL_Color{ 0, 0, 0, 255 }, "C", speed);
}
} // namespace

TEST_CASE("CollisionSystem::carVsCar separates two overlapping cars in the same lane", "[collision]") {
    std::vector<Car> cars;
    cars.push_back(makeCar(100.0f, 0.0f, 50.0f));
    cars.push_back(makeCar(105.0f, 0.0f, 60.0f)); // 5 units ahead, well inside carLength

    CollisionSystem::Config cfg{ 1000.0f, 30.0f, 16.0f };
    CollisionSystem::carVsCar(cars, cfg);

    REQUIRE(cars[0].speed == Catch::Approx(0.0f));
    REQUIRE(cars[1].speed == Catch::Approx(0.0f));
    REQUIRE(cars[0].recoveryTimer == Catch::Approx(Car::kRecoveryDuration));
    REQUIRE(cars[1].recoveryTimer == Catch::Approx(Car::kRecoveryDuration));
    // The trailing/leading pair should end up farther apart than they started.
    REQUIRE(std::abs(cars[1].s - cars[0].s) > 5.0f);
}

TEST_CASE("CollisionSystem::carVsCar ignores cars that are far apart", "[collision]") {
    std::vector<Car> cars;
    cars.push_back(makeCar(0.0f, 0.0f, 50.0f));
    cars.push_back(makeCar(500.0f, 0.0f, 60.0f));

    CollisionSystem::Config cfg{ 1000.0f, 30.0f, 16.0f };
    CollisionSystem::carVsCar(cars, cfg);

    REQUIRE(cars[0].speed == Catch::Approx(50.0f));
    REQUIRE(cars[1].speed == Catch::Approx(60.0f));
}

TEST_CASE("CollisionSystem::carVsCar ignores cars overlapping in s but in different lanes", "[collision]") {
    std::vector<Car> cars;
    cars.push_back(makeCar(100.0f, -25.0f, 50.0f));
    cars.push_back(makeCar(102.0f, 25.0f, 60.0f)); // 50 units of lateral gap, carWidth=16

    CollisionSystem::Config cfg{ 1000.0f, 30.0f, 16.0f };
    CollisionSystem::carVsCar(cars, cfg);

    REQUIRE(cars[0].speed == Catch::Approx(50.0f));
    REQUIRE(cars[1].speed == Catch::Approx(60.0f));
}

TEST_CASE("CollisionSystem::carVsRock stops a car overlapping an active rock", "[collision]") {
    Track track(0.0f, 0.0f, 200.0f, 50.0f, 40.0f);
    std::vector<Car> cars;
    cars.push_back(makeCar(100.0f, 45.0f, 80.0f));
    std::vector<Rock> rocks;
    rocks.emplace_back(102.0f, 45.0f, track, 20.0f);

    CollisionSystem::Config cfg{ 1000.0f, 30.0f, 16.0f };
    CollisionSystem::carVsRock(cars, rocks, cfg);

    REQUIRE(cars[0].speed == Catch::Approx(0.0f));
    REQUIRE(cars[0].recoveryTimer == Catch::Approx(Car::kRecoveryDuration));
}

TEST_CASE("CollisionSystem::carVsRock ignores inactive rocks", "[collision]") {
    Track track(0.0f, 0.0f, 200.0f, 50.0f, 40.0f);
    std::vector<Car> cars;
    cars.push_back(makeCar(100.0f, 45.0f, 80.0f));
    std::vector<Rock> rocks;
    rocks.emplace_back(101.0f, 45.0f, track, 20.0f);
    rocks[0].active = false;

    CollisionSystem::Config cfg{ 1000.0f, 30.0f, 16.0f };
    CollisionSystem::carVsRock(cars, rocks, cfg);

    REQUIRE(cars[0].speed == Catch::Approx(80.0f));
}

TEST_CASE("CollisionSystem::carVsGate stops a car at a closed gate", "[collision]") {
    Track track(0.0f, 0.0f, 200.0f, 50.0f, 40.0f);
    std::vector<Car> cars;
    cars.push_back(makeCar(100.0f, 0.0f, 80.0f));

    std::vector<Gate> gates;
    for (int attempt = 0; attempt < 100; ++attempt) {
        gates.clear();
        gates.emplace_back(102.0f, track, 20.0f);
        if (gates[0].isClosed()) break;
    }
    REQUIRE(gates[0].isClosed());

    CollisionSystem::Config cfg{ 1000.0f, 30.0f, 16.0f };
    CollisionSystem::carVsGate(cars, gates, cfg);

    REQUIRE(cars[0].speed == Catch::Approx(0.0f));
    REQUIRE(cars[0].recoveryTimer == Catch::Approx(Car::kRecoveryDuration));
}

TEST_CASE("CollisionSystem::carVsGate ignores an open gate", "[collision]") {
    Track track(0.0f, 0.0f, 200.0f, 50.0f, 40.0f);
    std::vector<Car> cars;
    cars.push_back(makeCar(100.0f, 0.0f, 80.0f));

    std::vector<Gate> gates;
    for (int attempt = 0; attempt < 100; ++attempt) {
        gates.clear();
        gates.emplace_back(101.0f, track, 20.0f);
        if (!gates[0].isClosed()) break;
    }
    REQUIRE_FALSE(gates[0].isClosed());

    CollisionSystem::Config cfg{ 1000.0f, 30.0f, 16.0f };
    CollisionSystem::carVsGate(cars, gates, cfg);

    REQUIRE(cars[0].speed == Catch::Approx(80.0f));
}

TEST_CASE("CollisionSystem::resolveAll runs all three passes without crashing", "[collision]") {
    Track track(0.0f, 0.0f, 200.0f, 50.0f, 40.0f);
    std::vector<Car> cars;
    cars.push_back(makeCar(100.0f, 0.0f, 80.0f));
    cars.push_back(makeCar(103.0f, 0.0f, 70.0f));
    std::vector<Rock> rocks;
    rocks.emplace_back(200.0f, 45.0f, track, 20.0f);
    std::vector<Gate> gates;
    gates.emplace_back(300.0f, track, 20.0f);

    CollisionSystem::Config cfg{ 1000.0f, 30.0f, 16.0f };
    REQUIRE_NOTHROW(CollisionSystem::resolveAll(cars, rocks, gates, cfg));
}
