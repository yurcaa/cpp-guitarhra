#pragma once
#include <SFML/Graphics.hpp>

inline constexpr float WIN_W = 560.f;
inline constexpr float WIN_H = 720.f;

inline constexpr int   COLS  = 3;
inline constexpr float COL_W = 115.f;
inline constexpr float COL_X[3] = { 15.f, 135.f, 255.f };

// SEP_X[i] = COL_X[i] + COL_W so columns touch the separators with no gap
inline constexpr float SEP_W    = 5.f;
inline constexpr float SEP_X[2] = { 130.f, 250.f };

inline const sf::Color COL_COLOR[3] = {
    sf::Color( 60, 200,  80),
    sf::Color(200,  60,  60),
    sf::Color( 60, 120, 220),
};
inline constexpr char COL_KEY[3] = { 'A', 'S', 'D' };

inline constexpr float BLOCK_H  = 50.f;
inline constexpr float TARGET_Y = 600.f;
inline constexpr float TARGET_H = 60.f;
inline constexpr float HIT_WIN  = 52.f;

inline constexpr float SPEED_START = 180.f;
inline constexpr float SPEED_MAX   = 420.f;
inline constexpr float SPEED_STEP  =  25.f;
inline constexpr float SPEED_EVERY =   8.f;

inline constexpr float PANEL_X  = 380.f;
inline constexpr float PANEL_CX = PANEL_X + (WIN_W - PANEL_X) / 2.f;
