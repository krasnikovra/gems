#include "Brush.h"
#include "Resources.h"
#include "Map.h"
#include <iostream>

constexpr unsigned GEMS_BRUSHED_COUNT = 2;

Brush::Brush(Map& parent, const Color& color) : Gem(parent, color, BRUSH_TEXTURE_PATH) {}

void Brush::onDeath() {
    std::vector<sf::Vector2u> gemsToBrush;
    while (gemsToBrush.size() < GEMS_BRUSHED_COUNT) {
        sf::Vector2u randPos = sf::Vector2u(rand() % _map.getUVSize().x, rand() % _map.getUVSize().y);
        bool isSelf = randPos == _uv;
        bool isUnique = std::find(gemsToBrush.begin(), gemsToBrush.end(), randPos) == gemsToBrush.end();
        bool isNeighbour = abs(static_cast<int>(randPos.x) - static_cast<int>(_uv.x)) +
            abs(static_cast<int>(randPos.y) - static_cast<int>(_uv.y)) < 2;
        if (!isSelf && isUnique && !isNeighbour)
            gemsToBrush.push_back(randPos);
    }
    for (auto& pos : gemsToBrush)
        if (_map.getGem(pos))
            _map.getGem(pos)->setColor(_color);
}
