#pragma once

#include "Gem.h"

class Bomb : public Gem {
public:
    explicit Bomb() = delete;
    explicit Bomb(Map& parent, const Color& color);
    explicit Bomb(const Bomb& other) = default;
    explicit Bomb(Bomb&& other) = default;
    virtual ~Bomb() = default;
    virtual void onDeath() override;
};
