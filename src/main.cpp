#include "Game.h"
#include <iostream>

#define BAD_TEXTURE_LINK_OUT -1337

int main() {
    try {
        Game gems(1200, 700);
        return gems.run();
    }
    catch (const NoGemTextureException& e) {
        std::cout << e.what() << std::endl;
        return BAD_TEXTURE_LINK_OUT;
    }
}
