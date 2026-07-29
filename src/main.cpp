#include "game/Game.h"

int main(int /*argc*/, char* /*argv*/[]) {
    Game game;
    if (!game.init(1600, 900, "miniCar")) return 1;
    return game.run();
}
