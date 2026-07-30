#include <catch2/catch_all.hpp>
#include <vector>

#include "actor/Car.h"
#include "game/DriverAssignmentService.h"

namespace {
std::vector<Car> makeCars(std::size_t count) {
    std::vector<Car> cars;
    cars.reserve(count);
    for (std::size_t i = 0; i < count; ++i) {
        cars.emplace_back(0.0f, 0.0f, 50.0f, SDL_Color{ 0, 0, 0, 255 }, "C", 50.0f);
    }
    return cars;
}
} // namespace

TEST_CASE("DriverAssignmentService::assignPlayer1 picks a valid index and sets Player1", "[driver_assignment]") {
    auto cars = makeCars(6);
    std::size_t index = DriverAssignmentService::assignPlayer1(cars);

    REQUIRE(index < cars.size());
    REQUIRE(cars[index].driver() == DriverKind::Player1);
}

TEST_CASE("DriverAssignmentService::assignPlayer2 never picks the Player1 index", "[driver_assignment]") {
    auto cars = makeCars(6);
    std::size_t p1 = DriverAssignmentService::assignPlayer1(cars);
    auto p2 = DriverAssignmentService::assignPlayer2(cars, p1);

    REQUIRE(p2.has_value());
    REQUIRE(*p2 != p1);
    REQUIRE(cars[*p2].driver() == DriverKind::Player2);
}

TEST_CASE("DriverAssignmentService::assignPlayer2 returns nullopt with no AI cars left", "[driver_assignment]") {
    auto cars = makeCars(2);
    cars[0].setDriver(DriverKind::Player1);
    cars[1].setDriver(DriverKind::Player2);

    auto p2 = DriverAssignmentService::assignPlayer2(cars, 0);
    REQUIRE_FALSE(p2.has_value());
}

TEST_CASE("DriverAssignmentService::removePlayer2 reverts the car to AI", "[driver_assignment]") {
    auto cars = makeCars(6);
    std::size_t p1 = DriverAssignmentService::assignPlayer1(cars);
    auto p2 = DriverAssignmentService::assignPlayer2(cars, p1);
    REQUIRE(p2.has_value());

    bool ok = DriverAssignmentService::removePlayer2(cars, *p2);
    REQUIRE(ok);
    REQUIRE(cars[*p2].driver() == DriverKind::Ai);
}

TEST_CASE("DriverAssignmentService::removePlayer2 fails for an index that isn't Player2", "[driver_assignment]") {
    auto cars = makeCars(6);
    std::size_t p1 = DriverAssignmentService::assignPlayer1(cars);

    bool ok = DriverAssignmentService::removePlayer2(cars, p1);
    REQUIRE_FALSE(ok);
    REQUIRE(cars[p1].driver() == DriverKind::Player1);
}
