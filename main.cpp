#include "include/Core/Game.h"
#include <iostream>

int main() {
    try {
        std::cout << "=== Dungeon Crawler v2.0 ===" << std::endl;
        std::cout << "Starting game..." << std::endl;

        Game game;
        game.run();

        std::cout << "Game ended normally" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown error occurred" << std::endl;
        return 1;
    }

    return 0;
}
