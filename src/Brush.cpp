#include "Brush.h"
#include "Resources.h"
#include "Map.h"
#include <iostream>

constexpr unsigned GEMS_BRUSHED_COUNT = 2;

Brush::Brush(Map& parent, const Color& color) : Gem(parent, color, BRUSH_TEXTURE_PATH) {}

void Brush::onDeath() {
    std::cout << "brush is used" << std::endl;
    std::vector<sf::Vector2u> gemsToBrush;
    while (gemsToBrush.size() < GEMS_BRUSHED_COUNT) {
        sf::Vector2u randPos = sf::Vector2u(rand() % _map->getUVSize().x, rand() % _map->getUVSize().y);
        bool isSelf = randPos == sf::Vector2u(_u, _v);
        bool isUnique = std::find(gemsToBrush.begin(), gemsToBrush.end(), randPos) == gemsToBrush.end();
        bool isNeighbour = abs(static_cast<int>(randPos.x) - static_cast<int>(_u)) +
            abs(static_cast<int>(randPos.y) - static_cast<int>(_v)) < 2;
        bool hasAnotherColor = _map->getGem(randPos) ? _map->getGem(randPos)->getColor() != _color : true;
        if (!isSelf && isUnique && !isNeighbour)
            gemsToBrush.push_back(randPos);
    }
    for (auto& pos : gemsToBrush)
        if (_map->getGem(pos))
            _map->getGem(pos)->setColor(_color);
}
