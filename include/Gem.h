#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Color.hpp>

class Map;

class Gem {
public:
    enum class Color {
        Green,
        Red,
        Yellow,
        Cyan,
        Blue,
        Count
    };
    static Color RandomColor();

    explicit Gem() = delete;
    explicit Gem(Map& parent, const Color& color);
    explicit Gem(const Gem& other) = default;
    explicit Gem(Gem&& other) = default;
    virtual ~Gem() = default;
    void setPosition(const sf::Vector2f& pos);
    void setColor(const Color& color);
    void bindMapCell(const sf::Vector2u& cellUV);
    float getWidth() const;
    float getHeight() const;
    unsigned getU() const;
    unsigned getV() const;
    sf::Vector2u getUV() const;
    sf::Vector2f getPosition() const;
    Color getColor() const;
    void draw();
    void select();
    void unselect();
    bool isSelected() const;
    bool operator==(const Gem& other) const;
    virtual void onDeath();
protected:
    explicit Gem(Map& parent, const Color& color, const std::string& texturePath);
    float _calculateScale(const float margin) const;
    void _setScale(const float scale);
    void _fixSize();
    void _fixSprite();

    Map* _map;
    sf::Sprite _sprite;
    sf::Texture _texture;
    Color _color;
    float _width, _height;
    float _scaleDefault, _scaleSelected;
    float _scale;
    unsigned _u, _v;
};

class NoGemTextureException : public std::exception {
public:
    NoGemTextureException(const std::string& texturePath);
    NoGemTextureException(const NoGemTextureException& other);
    virtual const char* what() const noexcept override;
    ~NoGemTextureException();
private:
    char* _msg;
    int _msglen;
};
