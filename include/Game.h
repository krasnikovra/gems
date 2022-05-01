#pragma once
#include <SFML/Graphics.hpp>
#include "Map.h"

class Game {
public:
    explicit Game();
    explicit Game(const unsigned winWidth, const unsigned winHeight);
    explicit Game(const Game& other) = delete;
    explicit Game(Game&& other) noexcept = delete;
    ~Game() = default;
    std::unique_ptr<sf::RenderWindow>& getWindow();
    int run();
private:
    std::unique_ptr<sf::RenderWindow> _window;
    std::unique_ptr<Map> _map;
};
