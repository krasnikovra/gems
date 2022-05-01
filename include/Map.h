#pragma once

#include <SFML/Graphics.hpp>
#include "Gem.h"

class Game;

class Map {
public:
    explicit Map() = default;
    explicit Map(const Map& other) = delete;
    explicit Map(Map&& other) noexcept = delete;
    explicit Map(Game& parent);
    ~Map() = default;
    void draw();
    Game* getGame() const;
    float getCellWidth() const;
    float getCellHeight() const;
    float getX0() const;
    float getY0() const;
    sf::Vector2u getUVSize() const;
    std::unique_ptr<Gem>& getGem(const unsigned u, const unsigned v);
    std::unique_ptr<Gem>& getGem(const sf::Vector2u& uv);
    bool inMap(const sf::Vector2i& pos) const;
    void handleEvent(const sf::Event& event);
private:
    void _drawGrid();
    void _drawGems();
    void _swapGems(const sf::Vector2u& uv1, const sf::Vector2u& uv2);
    float _xGrid(const unsigned i) const;
    float _yGrid(const unsigned i) const;
    bool _destroyCombos(bool inGame);
    bool _pushDown();
    bool _fillGaps();
    void _startPushDownTimer();
    void _stopPushDownTimer();
    void _startSwapTimer();
    void _stopSwapTimer();
    void _update();

    float _x0, _y0;
    float _cellWidth, _cellHeight;
    unsigned _u, _v;
    Game* _game;
    Gem* _gemSelected;
    std::vector<std::unique_ptr<Gem>> _gems;
    float _pushDownTimer;
    bool _isPushDownTimerActive;
    float _swapTimer;
    bool _isSwapTimerActive;
    std::pair<sf::Vector2u, sf::Vector2u> _lastSwap;
    sf::Clock _clock;
};
