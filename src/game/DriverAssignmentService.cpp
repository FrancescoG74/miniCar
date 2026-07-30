#include "game/DriverAssignmentService.h"

#include <random>
#include <vector>

#include "actor/Car.h"

namespace {

std::mt19937& randomEngine() {
    static std::mt19937 engine{ std::random_device{}() };
    return engine;
}

} // namespace

std::size_t DriverAssignmentService::assignPlayer1(std::vector<Car>& cars) {
    return assignPlayer1(cars, randomEngine());
}

std::size_t DriverAssignmentService::assignPlayer1(std::vector<Car>& cars, std::mt19937& rng) {
    std::uniform_int_distribution<std::size_t> distribution(0, cars.size() - 1);
    const std::size_t index = distribution(rng);
    cars[index].setDriver(DriverKind::Player1);
    return index;
}

std::optional<std::size_t> DriverAssignmentService::assignPlayer2(
    std::vector<Car>& cars, std::size_t player1Index) {
    return assignPlayer2(cars, player1Index, randomEngine());
}

std::optional<std::size_t> DriverAssignmentService::assignPlayer2(
    std::vector<Car>& cars, std::size_t player1Index, std::mt19937& rng) {
    std::vector<std::size_t> candidates;
    for (std::size_t index = 0; index < cars.size(); ++index) {
        if (index != player1Index && cars[index].driver() == DriverKind::Ai) {
            candidates.push_back(index);
        }
    }
    if (candidates.empty()) return std::nullopt;

    std::uniform_int_distribution<std::size_t> distribution(0, candidates.size() - 1);
    const std::size_t index = candidates[distribution(rng)];
    cars[index].setDriver(DriverKind::Player2);
    return index;
}

bool DriverAssignmentService::removePlayer2(std::vector<Car>& cars, std::size_t player2Index) {
    if (player2Index >= cars.size() || cars[player2Index].driver() != DriverKind::Player2) {
        return false;
    }
    cars[player2Index].setDriver(DriverKind::Ai);
    return true;
}
