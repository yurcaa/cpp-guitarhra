#pragma once
#include <SFML/Graphics.hpp>
#include <algorithm>
#include "Constants.h"

inline sf::Color withAlpha(sf::Color c, sf::Uint8 a)
{
    c.a = a;
    return c;
}

// letterbox: keeps aspect ratio on any screen size
inline sf::View computeView(unsigned sw, unsigned sh)
{
    float scale = std::min(sw / WIN_W, sh / WIN_H);
    float vpW   = (WIN_W * scale) / sw;
    float vpH   = (WIN_H * scale) / sh;
    sf::View v(sf::FloatRect(0.f, 0.f, WIN_W, WIN_H));
    v.setViewport(sf::FloatRect((1.f - vpW) / 2.f, (1.f - vpH) / 2.f, vpW, vpH));
    return v;
}
