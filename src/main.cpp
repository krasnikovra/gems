#include "Game.h"
#include <iostream>

constexpr int FAIL = -1;

int main() {
    try {
        Game gems(1200, 700);
        return gems.run();
    }
    catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        return FAIL;
    }
}
