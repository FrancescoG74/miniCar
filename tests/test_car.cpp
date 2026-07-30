#include <catch2/catch_all.hpp>
#include <SDL2/SDL.h>
#include <string>
#include <vector>

#include "actor/Car.h"

namespace {

// A CarControls with no track/keys/blocked list; AI cars alone in `allCars`
// with laneOffset == targetLaneOffset and speed == targetSpeed stay perfectly
// deterministic frame to frame (see AiInput::drive).
CarControls deterministicControls() {
    CarControls ctrl;
    ctrl.keys = nullptr;
    ctrl.track = nullptr;
    ctrl.blockedSPositions = nullptr;
    return ctrl;
}

} // namespace

TEST_CASE("Car default constructor starts idle at the origin of the track", "[car]") {
    Car car;
    REQUIRE(car.s == Catch::Approx(0.0f));
    REQUIRE(car.speed == Catch::Approx(0.0f));
    REQUIRE(car.laps == 0);
    REQUIRE(car.driver() == DriverKind::Ai);
}

TEST_CASE("Car parameterized constructor stores all given fields", "[car]") {
    SDL_Color color{ 1, 2, 3, 255 };
    Car car(10.0f, 5.0f, 90.0f, color, "Tester", 90.0f);
    REQUIRE(car.s == Catch::Approx(10.0f));
    REQUIRE(car.laneOffset == Catch::Approx(5.0f));
    REQUIRE(car.targetLaneOffset == Catch::Approx(5.0f));
    REQUIRE(car.speed == Catch::Approx(90.0f));
    REQUIRE(car.targetSpeed == Catch::Approx(90.0f));
    REQUIRE(std::string(car.getName()) == "Tester");
}

TEST_CASE("Car starting behind the line has not started yet", "[car]") {
    Car car(-45.0f, 0.0f, 90.0f, SDL_Color{ 0, 0, 0, 255 }, "Grid", 90.0f);
    REQUIRE_FALSE(car.hasStarted);
}

TEST_CASE("Car starting at or after the line has already started", "[car]") {
    Car car(0.0f, 0.0f, 90.0f, SDL_Color{ 0, 0, 0, 255 }, "Grid", 90.0f);
    REQUIRE(car.hasStarted);
}

TEST_CASE("Car::update advances s by speed*dt when it does not cross the line", "[car]") {
    std::vector<Car> cars;
    cars.emplace_back(0.0f, 0.0f, 50.0f, SDL_Color{ 0, 0, 0, 255 }, "Solo", 50.0f);

    CarControls ctrl = deterministicControls();
    cars[0].update(1.0f, cars, 0, 1000.0f, ctrl);

    REQUIRE(cars[0].s == Catch::Approx(50.0f));
    REQUIRE(cars[0].distanceTraveled == Catch::Approx(50.0f));
    REQUIRE(cars[0].laps == 0);
}

TEST_CASE("Car::update wraps s and credits a lap once it has already started", "[car]") {
    std::vector<Car> cars;
    cars.emplace_back(90.0f, 0.0f, 20.0f, SDL_Color{ 0, 0, 0, 255 }, "Solo", 20.0f);

    CarControls ctrl = deterministicControls();
    cars[0].update(1.0f, cars, 0, 100.0f, ctrl);

    REQUIRE(cars[0].s == Catch::Approx(10.0f));
    REQUIRE(cars[0].laps == 1);
}

TEST_CASE("Car::update's first crossing of the line does not count as a lap", "[car]") {
    // Grid cars spawn behind s=0 (hasStarted=false); the first crossing just
    // brings them up to the start line and must not itself be a completed lap.
    std::vector<Car> cars;
    cars.emplace_back(-10.0f, 0.0f, 20.0f, SDL_Color{ 0, 0, 0, 255 }, "Grid", 20.0f);

    CarControls ctrl = deterministicControls();
    // -10 + 20*2 = 30, which crosses the totalLength=20 line exactly once.
    cars[0].update(2.0f, cars, 0, 20.0f, ctrl);

    REQUIRE(cars[0].hasStarted);
    REQUIRE(cars[0].laps == 0);
    REQUIRE(cars[0].s == Catch::Approx(10.0f));
}

TEST_CASE("Car::createInitialGrid returns six cars staggered behind the start line", "[car]") {
    auto grid = Car::createInitialGrid(25.0f);
    REQUIRE(grid.size() == 6);
    for (const auto& car : grid) {
        REQUIRE(car.s < 0.0f);
        REQUIRE_FALSE(car.hasStarted);
        REQUIRE(car.driver() == DriverKind::Ai);
    }
}

TEST_CASE("Car::setDriver switches to keyboard control and back to AI", "[car]") {
    Car car;
    car.setDriver(DriverKind::Player1);
    REQUIRE(car.driver() == DriverKind::Player1);

    car.setDriver(DriverKind::Ai);
    REQUIRE(car.driver() == DriverKind::Ai);
}

TEST_CASE("KeyboardInput accelerates and steers a Player1 car via CarControls", "[car]") {
    std::vector<Car> cars;
    cars.emplace_back(0.0f, 0.0f, 0.0f, SDL_Color{ 0, 0, 0, 255 }, "P1", 0.0f);
    cars[0].setDriver(DriverKind::Player1);

    std::vector<Uint8> keys(SDL_NUM_SCANCODES, 0);
    keys[SDL_SCANCODE_W] = 1; // accelerate (WASD scheme)
    keys[SDL_SCANCODE_D] = 1; // steer right

    CarControls ctrl;
    ctrl.keys = keys.data();
    ctrl.maxSpeed = 130.0f;
    ctrl.playerAccel = 60.0f;
    ctrl.playerSteerRate = 70.0f;
    ctrl.laneLimit = 53.0f;
    ctrl.track = nullptr;

    cars[0].update(1.0f, cars, 0, 1000.0f, ctrl);

    REQUIRE(cars[0].speed == Catch::Approx(60.0f));
    // Steer rate (70) exceeds the lane limit (53), so the offset clamps rather
    // than overshooting.
    REQUIRE(cars[0].laneOffset == Catch::Approx(53.0f));
}
