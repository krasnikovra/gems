#include "Game.h"
#include <iostream>

#define GAME_NAME "Gems"

Game::Game() : _window(std::make_unique<sf::RenderWindow>(sf::VideoMode(800, 600), GAME_NAME)),
    _map(std::make_unique<Map>(*this)) {}

Game::Game(const unsigned winWidth, const unsigned winHeight) :
    _window(std::make_unique<sf::RenderWindow>(sf::VideoMode(winWidth, winHeight), GAME_NAME)),
    _map(std::make_unique<Map>(*this)) {}

std::unique_ptr<sf::RenderWindow>& Game::getWindow() {
    return _window;
}

int Game::run() {
    while (_window->isOpen()) {
        sf::Event event;
        while (_window->pollEvent(event)) {
            switch (event.type) {
            case sf::Event::Closed:
                _window->close();
                return 0;
            case sf::Event::MouseButtonPressed:
                if (_map->inMap(sf::Mouse::getPosition(*_window)))
                    _map->handleEvent(event);
                else
                    std::cout << "clicked somewhere but not the game map" << std::endl;
            }       
        }
        _window->clear(sf::Color::White);
        _map->draw();
        _window->display();
    }
    return 0;
}
