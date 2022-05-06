#include "Map.h"
#include "Game.h"
#include "Bomb.h"
#include <iostream>

constexpr unsigned U = 7;
constexpr unsigned V = 7;
constexpr float MARGINX_RATIO = .1f;
constexpr float MARGINY_RATIO = .05f;
constexpr float PUSH_DOWN_TIMER_MAX = 0.5f;
constexpr float SWAP_TIMER_MAX = 0.25f;

Map::Map(Game& parent) : _game(parent), _uvSize(sf::Vector2u(U, V)),
    _gems(std::vector<std::unique_ptr<Gem>>(U * V)) {
    _pos.x = _game.getWindow()->getSize().x * MARGINX_RATIO;
    _pos.y = _game.getWindow()->getSize().y * MARGINY_RATIO;
    const float marginx = _pos.x; // horizontal align: center
    const float marginy = _pos.y; // vertical align: center
    _cellSize.x = (_game.getWindow()->getSize().x - 2 * marginx) / static_cast<float>(_uvSize.x);
    _cellSize.y = (_game.getWindow()->getSize().y - 2 * marginy) / static_cast<float>(_uvSize.y);
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

Game& Map::getGame() const {
    return _game;
}

sf::Vector2f Map::getCellSize() const {
    return _cellSize;
}

sf::Vector2f Map::getPos() const {
    return _pos;
}

sf::Vector2u Map::getUVSize() const {
    return _uvSize;
}

std::unique_ptr<Gem>& Map::getGem(const unsigned u, const unsigned v) {
    return _gems[v * _uvSize.x + u];
}

std::unique_ptr<Gem>& Map::getGem(const sf::Vector2u& uv) {
    return _gems[uv.y * _uvSize.y + uv.x];
}

bool Map::inMap(const sf::Vector2i& pos) const {
    return (_Grid(0).x < pos.x && pos.x < _Grid(_uvSize.x).x) &&
        (_Grid(0).y < pos.y && pos.y < _Grid(_uvSize.y).y);
}

void Map::handleEvent(const sf::Event& event) {
    if (_isPushDownTimerActive || _isSwapTimerActive)
        return;
    const sf::Vector2i pos = sf::Mouse::getPosition(*_game.getWindow());
    switch (event.type) {
    case sf::Event::MouseButtonPressed:
        unsigned u, v;
        for (u = 0; u < _uvSize.x && pos.x > _Grid(u + 1).x; u++);
        for (v = 0; v < _uvSize.y && pos.y > _Grid(v + 1).y; v++);
        if (!getGem(u, v))
            return;
        if (getGem(u, v)->isSelected()) {
            getGem(u, v)->unselect();
            _gemSelected = nullptr;
        }
        else {
            if (!_gemSelected) {
                getGem(u, v)->select();
                _gemSelected = getGem(u, v).get();
            }
            else {
                // we have one already selected
                // check if new one is neighbour of selected
                _gemSelected->unselect();
                const sf::Vector2u clickedUV = sf::Vector2u(u, v);
                const sf::Vector2u selectedUV = _gemSelected->getUV();
                const bool areNeighbours = abs(static_cast<int>(u) - static_cast<int>(selectedUV.x)) +
                    abs(static_cast<int>(v) - static_cast<int>(selectedUV.y)) < 2;
                if (areNeighbours) {
                    _swapGems(selectedUV, clickedUV);
                    _lastSwap = std::make_pair(selectedUV, clickedUV);
                    _startSwapTimer();
                }
                _gemSelected = nullptr;
            }
        }
    }
}

void Map::_drawGrid() {
    for (unsigned i = 0; i < _uvSize.x + 1; i++) {
        sf::Vertex line[] = {
            sf::Vertex(sf::Vector2f(_Grid(i).x, _Grid(0).y), sf::Color::Black),
            sf::Vertex(sf::Vector2f(_Grid(i).x, _Grid(_uvSize.y).y), sf::Color::Black)
        };
        _game.getWindow()->draw(line, 2, sf::Lines);
    }
    for (unsigned i = 0; i < _uvSize.y + 1; i++) {
        sf::Vertex line[] = {
            sf::Vertex(sf::Vector2f(_Grid(0).x, _Grid(i).y), sf::Color::Black),
            sf::Vertex(sf::Vector2f(_Grid(_uvSize.x).x, _Grid(i).y), sf::Color::Black)
        };
        _game.getWindow()->draw(line, 2, sf::Lines);
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

sf::Vector2f Map::_Grid(const unsigned i) const {
    return _pos + static_cast<float>(i) * _cellSize;
}

bool Map::_destroyCombos(bool inGame) {
    bool res = false;
    std::vector<sf::Vector2u> combinedGemsPos;
    for (unsigned i = 0; i < _uvSize.x; i++)
        for (unsigned j = 0; j < _uvSize.y; j++) {
            if (getGem(i, j)) {
                bool isHorisontalCombo = 0 < i && i < _uvSize.x - 1 && getGem(i - 1, j) && getGem(i + 1, j) ?
                    *getGem(i - 1, j) == *getGem(i, j) && *getGem(i, j) == *getGem(i + 1, j) : false;
                bool isVerticalCombo = 0 < j && j < _uvSize.y - 1 && getGem(i, j - 1) && getGem(i, j + 1) ?
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
    for (unsigned i = 0; i < _uvSize.x; i++) {
        for (unsigned j = _uvSize.y - 1; j > 0; j--)
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
    for (unsigned i = 0; i < _uvSize.x; i++)
        for (unsigned j = 0; j < _uvSize.y; j++)
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
