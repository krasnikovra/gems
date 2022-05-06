#define _CRT_SECURE_NO_WARNINGS

#include "Gem.h"
#include "Map.h"
#include "Game.h"
#include "Resources.h"
#include "Bomb.h"
#include "Brush.h"
#include <iostream>

constexpr float CELL_MARGIN_RATIO = .2f;
constexpr float CELL_MARGIN_RATIO_SELECTED = .05f;
constexpr unsigned BONUS_RADIUS = 3;
constexpr float BONUS_PROB = 0.1f;

static std::map<Gem::Color, sf::Color> gcolorMap = {
    { Gem::Color::Green, sf::Color::Green },
    { Gem::Color::Red, sf::Color(255, 150, 150, 255)},
    { Gem::Color::Yellow, sf::Color(255, 255, 150, 255)},
    { Gem::Color::Cyan, sf::Color::Cyan },
    { Gem::Color::Blue, sf::Color(150, 150, 255, 255)}
};

Gem::Color Gem::RandomColor() {
    return static_cast<Gem::Color>(rand() % static_cast<int>(Color::Count));
}

Gem::Gem(Map& parent, const Color& color) : Gem(parent, color, GEM_TEXTURE_PATH) {}

Gem::Gem(Map& parent, const Color& color, const std::string& texturePath) : _map(parent), _color(color) {
    sf::Image img;
    if (!img.loadFromFile(texturePath))
        throw NoGemTextureException(texturePath);
    sf::Texture tex;
    tex.loadFromImage(img);
    _texture = tex;
    _sprite.setTexture(_texture);
    _sprite.setColor(gcolorMap[_color]);
    _scaleDefault = _calculateScale(CELL_MARGIN_RATIO);
    _scaleSelected = _calculateScale(CELL_MARGIN_RATIO_SELECTED);
    _scale = _scaleDefault;
    _sprite.setScale(_scale, _scale);
    _size = _scale * static_cast<sf::Vector2f>(tex.getSize());
}

void Gem::setPosition(const sf::Vector2f& pos) {
    _sprite.setPosition(pos);
}

void Gem::setColor(const Color& color) {
    _color = color;
    _sprite.setColor(gcolorMap[_color]);
}

void Gem::bindMapCell(const sf::Vector2u& cellUV) {
    _uv = cellUV;
    const float x0 = _map.getPos().x;
    const float y0 = _map.getPos().y;
    const float cellWidth = _map.getCellSize().x;
    const float cellHeight = _map.getCellSize().y;
    sf::Vector2f pos = sf::Vector2f(
        x0 + _uv.x * cellWidth + cellWidth / 2.0f - _size.x / 2.0f,
        y0 + _uv.y * cellHeight + cellHeight / 2.0f - _size.y / 2.0f
    );
    _sprite.setPosition(pos);
}

sf::Vector2f Gem::getSize() const {
    return _size;
}

sf::Vector2u Gem::getUV() const {
    return _uv;
}

sf::Vector2f Gem::getPosition() const {
    return _sprite.getPosition();
}

Gem::Color Gem::getColor() const {
    return _color;
}

void Gem::draw() {
    _map.getGame().getWindow()->draw(_sprite);
}

void Gem::select() {
    _setScale(_scaleSelected);
}

void Gem::unselect() {
    _setScale(_scaleDefault);
}

bool Gem::isSelected() const {
    return _scale == _scaleSelected;
}

bool Gem::operator==(const Gem& other) const {
    return _color == other._color;
}

void Gem::onDeath() {
    if (rand() / static_cast<float>(RAND_MAX) < BONUS_PROB) {
        auto min = [](int a, int b) -> int { return a < b ? a : b; };
        auto max = [](int a, int b) -> int { return a > b ? a : b; };
        unsigned uMin = static_cast<unsigned>(max(static_cast<int>(_uv.x) - static_cast<int>(BONUS_RADIUS), 0));
        unsigned vMin = static_cast<unsigned>(max(static_cast<int>(_uv.y) - static_cast<int>(BONUS_RADIUS), 0));
        unsigned uMax = static_cast<unsigned>(min(static_cast<int>(_uv.x) + static_cast<int>(BONUS_RADIUS), _map.getUVSize().x - 1));
        unsigned vMax = static_cast<unsigned>(min(static_cast<int>(_uv.y) + static_cast<int>(BONUS_RADIUS), _map.getUVSize().y - 1));
        while (true) {
            sf::Vector2u randPos = sf::Vector2u(rand() % (uMax - uMin + 1) + uMin, rand() % (vMax - vMin + 1) + vMin);
            bool isSelf = randPos == _uv;
            if (!isSelf) {
                Color color;
                if (_map.getGem(randPos))
                    color = _map.getGem(randPos)->getColor();
                else
                    color = RandomColor();
                if (rand() % 2)
                    _map.getGem(randPos) = std::make_unique<Bomb>(_map, color);
                else
                    _map.getGem(randPos) = std::make_unique<Brush>(_map, _color);
                _map.getGem(randPos)->bindMapCell(randPos);
                break;
            }
        }
    }
}

float Gem::_calculateScale(const float margin) const {
    const unsigned texWidth = _texture.getSize().x;
    const unsigned texHeight = _texture.getSize().y;
    const float cellWidth = _map.getCellSize().x;
    const float cellHeight = _map.getCellSize().y;
    return cellWidth > cellHeight ?
        (cellHeight - 2 * margin * cellHeight) / static_cast<float>(texHeight) :
        (cellWidth - 2 * margin * cellWidth) / static_cast<float>(texWidth);
}

void Gem::_setScale(const float scale) {
    _scale = scale;
    _fixSize();
    _fixSprite();
}

void Gem::_fixSize() {
    _size = _scale * static_cast<sf::Vector2f>(_texture.getSize());
}

void Gem::_fixSprite() {
    _sprite.setScale(_scale, _scale);
    bindMapCell(_uv);
}

NoGemTextureException::NoGemTextureException(const std::string& texturePath) {
    std::string strmsg = std::string("No gem texture found! Make sure ")
        + texturePath + std::string(" exists.");
    _msglen = strmsg.size() + 1;
    _msg = new char[_msglen];
    strcpy(_msg, strmsg.c_str());
    _msg[strmsg.size()] = '\0';
}

NoGemTextureException::NoGemTextureException(const NoGemTextureException& other) {
    _msglen = other._msglen;
    _msg = new char[_msglen];
    strcpy(_msg, other._msg);
}

const char* NoGemTextureException::what() const noexcept {
    return _msg;
}

NoGemTextureException::~NoGemTextureException() {
    delete[] _msg;
}
