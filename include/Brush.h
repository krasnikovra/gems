#pragma once

#include "Gem.h"

class Brush : public Gem {
public:
    explicit Brush() = delete;
    explicit Brush(Map& parent, const Color& color);
    explicit Brush(const Brush& other) = default;
    explicit Brush(Brush&& other) = default;
    virtual ~Brush() = default;
    virtual void onDeath() override;
};
