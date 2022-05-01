#include "Map.h"
#include "Game.h"
#include "Bomb.h"
#include <iostream>

constexpr unsigned U = 7;
constexpr unsigned V = 7;
constexpr float MARGINX_RATIO = .1f;
constexpr float MARGINBOT_RATIO = .05f;
constexpr float MARGINTOP_RATIO = .05f;
constexpr float PUSH_DOWN_TIMER_MAX = 0.5f;
constexpr float SWAP_TIMER_MAX = 0.25f;

Map::Map(Game& parent) : _game(&parent), _u(U), _v(V),
    _gems(std::vector<std::unique_ptr<Gem>>(_u * _v)) {
    _x0 = _game->getWindow()->getSize().x * MARGINX_RATIO;
    _y0 = _game->getWindow()->getSize().y * MARGINTOP_RATIO;
    const float marginx = _x0; // horizontal align: center
    const float marginy = _game->getWindow()->getSize().y * MARGINBOT_RATIO; // vertical align: bottom
    _cellWidth = (_game->getWindow()->getSize().x - 2 * marginx) / static_cast<float>(_u);
    _cellHeight = (_game->getWindow()->getSize().y - _y0 - marginy) / static_cast<float>(_v);
    try {
        do {
            _fillGaps();
        } while (_destroyCombos(false));
    }
    catch (const NoGemTextureException& e) {
        throw NoGemTextureException(e);
    }
}

void Map::draw() {
    _update();
    _drawGrid();
    _drawGems();
}

Game* Map::getGame() const {
    return _game;
}

float Map::getCellWidth() const {
    return _cellWidth;
}

float Map::getCellHeight() const {
    return _cellHeight;
}

float Map::getX0() const {
    return _x0;
}

float Map::getY0() const {
    return _y0;
}

sf::Vector2u Map::getUVSize() const {
    return sf::Vector2u(_u, _v);
}

std::unique_ptr<Gem>& Map::getGem(const unsigned u, const unsigned v) {
    return _gems[v * _u + u];
}

std::unique_ptr<Gem>& Map::getGem(const sf::Vector2u& uv) {
    return _gems[uv.y * _u + uv.x];
}

bool Map::inMap(const sf::Vector2i& pos) const {
    return (_xGrid(0) < pos.x && pos.x < _xGrid(_u)) &&
        (_yGrid(0) < pos.y && pos.y < _yGrid(_v));
}

void Map::handleEvent(const sf::Event& event) {
    if (_isPushDownTimerActive || _isSwapTimerActive)
        return;
    sf::Vector2i pos = sf::Mouse::getPosition(*_game->getWindow());
    switch (event.type) {
    case sf::Event::MouseButtonPressed:
        unsigned u, v;
        for (u = 0; u < _u && pos.x > _xGrid(u + 1); u++);
        for (v = 0; v < _v && pos.y > _yGrid(v + 1); v++);
        if (!getGem(u, v))
            return;
        if (getGem(u, v)->isSelected()) {
            getGem(u, v)->unselect();
            _gemSelected = nullptr;
        }
        else {
            if (!_gemSelected) {
                getGem(u, v)->select();
                _gemSelected = &*getGem(u, v);
            }
            else {
                // we have one already selected
                // check if new one is neighbour of selected
                _gemSelected->unselect();
                sf::Vector2u clickedUV = sf::Vector2u(u, v);
                sf::Vector2u selectedUV = sf::Vector2u(_gemSelected->getU(), _gemSelected->getV());
                bool areNeighbours = abs(static_cast<int>(u) - static_cast<int>(_gemSelected->getU())) +
                    abs(static_cast<int>(v) - static_cast<int>(_gemSelected->getV())) < 2;
                if (areNeighbours) {
                    _swapGems(selectedUV, sf::Vector2u(u, v));
                    _lastSwap = std::make_pair(selectedUV, clickedUV);
                    _startSwapTimer();
                }
                _gemSelected = nullptr;
            }
        }
    }
}

void Map::_drawGrid() {
    for (unsigned i = 0; i < _u + 1; i++) {
        sf::Vertex line[] = {
            sf::Vertex(sf::Vector2f(_xGrid(i), _yGrid(0)), sf::Color::Black),
            sf::Vertex(sf::Vector2f(_xGrid(i), _yGrid(_v)), sf::Color::Black)
        };
        _game->getWindow()->draw(line, 2, sf::Lines);
    }
    for (unsigned i = 0; i < _v + 1; i++) {
        sf::Vertex line[] = {
            sf::Vertex(sf::Vector2f(_xGrid(0), _yGrid(i)), sf::Color::Black),
            sf::Vertex(sf::Vector2f(_xGrid(_u), _yGrid(i)), sf::Color::Black)
        };
        _game->getWindow()->draw(line, 2, sf::Lines);
    }
}

void Map::_drawGems() {
    for (auto& gem : _gems)
        if (gem) 
            gem->draw();
}

void Map::_swapGems(const sf::Vector2u& uv1, const sf::Vector2u& uv2) {
    if (getGem(uv1))
        getGem(uv1)->bindMapCell(uv2);
    if (getGem(uv2))
        getGem(uv2)->bindMapCell(uv1);
    
    std::unique_ptr<Gem> temp = std::move(getGem(uv1));
    getGem(uv1) = std::move(getGem(uv2));
    getGem(uv2) = std::move(temp);
}

float Map::_xGrid(const unsigned i) const {
    return _x0 + i * _cellWidth;
}

float Map::_yGrid(const unsigned i) const {
    return _y0 + i * _cellHeight;
}

bool Map::_destroyCombos(bool inGame) {
    bool res = false;
    std::vector<sf::Vector2u> combinedGemsPos;
    for (unsigned i = 0; i < _u; i++)
        for (unsigned j = 0; j < _v; j++) {
            if (getGem(i, j)) {
                bool isHorisontalCombo = 0 < i && i < _u - 1 && getGem(i - 1, j) && getGem(i + 1, j) ?
                    *getGem(i - 1, j) == *getGem(i, j) && *getGem(i, j) == *getGem(i + 1, j) : false;
                bool isVerticalCombo = 0 < j && j < _v - 1 && getGem(i, j - 1) && getGem(i, j + 1) ?
                    *getGem(i, j - 1) == *getGem(i, j) && *getGem(i, j) == *getGem(i, j + 1) : false;
                if (isHorisontalCombo) {
                    res = true;
                    combinedGemsPos.push_back(sf::Vector2u(i - 1, j));
                    combinedGemsPos.push_back(sf::Vector2u(i, j));
                    combinedGemsPos.push_back(sf::Vector2u(i + 1, j));
                }
                if (isVerticalCombo) {
                    res = true;
                    combinedGemsPos.push_back(sf::Vector2u(i, j - 1));
                    combinedGemsPos.push_back(sf::Vector2u(i, j));
                    combinedGemsPos.push_back(sf::Vector2u(i, j + 1));
                }
            }
        }
    for (auto& pos : combinedGemsPos) {
        if (getGem(pos)) {
            if (inGame)
                getGem(pos)->onDeath();
            getGem(pos) = nullptr;
        }
    }
    return res;
}

bool Map::_pushDown() {
    bool res = false;
    for (unsigned i = 0; i < _u; i++) {
        for (unsigned j = _v - 1; j > 0; j--)
            if (!getGem(i, j)) {
                // if gem pushed down then pushDown should be true
                if (getGem(i, j - 1))
                    res = true;
                _swapGems(sf::Vector2u(i, j), sf::Vector2u(i, j - 1));
            }
    }
    return res;
}

bool Map::_fillGaps() {
    bool res = false;
    for (unsigned i = 0; i < _u; i++)
        for (unsigned j = 0; j < _v; j++)
            if (!getGem(i, j)) {
                res = true;
                getGem(i, j) = std::make_unique<Gem>(*this, Gem::RandomColor());
                getGem(i, j)->bindMapCell(sf::Vector2u(i, j));
            }
    return res;
}

void Map::_startPushDownTimer() {
    _pushDownTimer = 0.0f;
    _isPushDownTimerActive = true;
}

void Map::_stopPushDownTimer() {
    _pushDownTimer = 0.0f;
    _isPushDownTimerActive = false;
}

void Map::_startSwapTimer() {
    _swapTimer = 0.0f;
    _isSwapTimerActive = true;
}

void Map::_stopSwapTimer() {
    _swapTimer = 0.0f;
    _isSwapTimerActive = false;
}

void Map::_update() {
    float timerStep = _clock.getElapsedTime().asSeconds();
    _clock.restart();
    if (_isPushDownTimerActive) {
        _pushDownTimer += timerStep;
        if (_pushDownTimer > PUSH_DOWN_TIMER_MAX) {
            if (_pushDown())
                _startPushDownTimer();
            else {
                _stopPushDownTimer();
                if (_destroyCombos(true)) {
                    _startPushDownTimer();
                }
            }
        }
    }
    else {
        if (_fillGaps())
            _startPushDownTimer();
    }
    if (_isSwapTimerActive) {
        _swapTimer += timerStep;
        if (_swapTimer > SWAP_TIMER_MAX) {
            _stopSwapTimer();
            if (_destroyCombos(true))
                _startPushDownTimer();
            else {
                _swapGems(_lastSwap.first, _lastSwap.second);
                std::cout << "wrong move" << std::endl;
            }
        }
    }
}
