#pragma once

#include <cstddef>
#include <optional>
#include <vector>

class Car;

// Chooses and changes who drives each car. It owns assignment policy only;
// platform UI (window titles) remains in Game, and Car owns controller creation.
class DriverAssignmentService {
public:
    static std::size_t assignPlayer1(std::vector<Car>& cars);
    static std::optional<std::size_t> assignPlayer2(std::vector<Car>& cars,
                                                     std::size_t player1Index);
    static bool removePlayer2(std::vector<Car>& cars, std::size_t player2Index);
};
