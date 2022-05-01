#include "Bomb.h"
#include "Resources.h"
#include "Map.h"
#include <iostream>

constexpr unsigned GEMS_BLOWN_COUNT = 4;

Bomb::Bomb(Map& parent, const Color& color) : Gem(parent, color, BOMB_TEXTURE_PATH) {}

void Bomb::onDeath() {
    std::cout << "bomb explode" << std::endl;
    std::vector<sf::Vector2u> gemsToBlow;
    while (gemsToBlow.size() < GEMS_BLOWN_COUNT) {
        sf::Vector2u randPos = sf::Vector2u(rand() % _map->getUVSize().x, rand() % _map->getUVSize().y);
        bool isSelf = randPos == sf::Vector2u(_u, _v);
        bool isUnique = std::find(gemsToBlow.begin(), gemsToBlow.end(), randPos) == gemsToBlow.end();
        if (!isSelf && isUnique)
            gemsToBlow.push_back(randPos);
    }
    for (auto& pos : gemsToBlow) {
        if (_map->getGem(pos))
            _map->getGem(pos)->onDeath();
        _map->getGem(pos) = nullptr;
    }
}
