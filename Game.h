#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <random>
#include <string>
#include "Block.h"

enum class GameState { Waiting, Countdown, Playing, GameOver };

struct FloatText
{
    std::string text;
    float x, y;
    float alpha = 255.f;
    float vy    = -80.f;
};

class Game
{
public:
    explicit Game(sf::Font& font);
    void handleEvent(const sf::Event& event);
    void update(float dt);
    void draw(sf::RenderWindow& window);

private:
    sf::Font&       font;
    sf::Texture     heartTex;
    sf::Sprite      heartSpr;
    float           heartScale = 1.f;

    sf::SoundBuffer popBuf;
    sf::Sound       popSound;
    sf::SoundBuffer lossBuf;
    sf::Sound       lossSound;

    GameState state          = GameState::Waiting;
    float     countdownTimer = 0.f;

    int   lives       = 3;
    int   score       = 0;
    int   combo       = 0;
    float speed       = 0.f;

    float lostLifeTimer = 0.f;
    int   lostLifeCount = 0;

    std::vector<Block>     blocks;
    std::vector<FloatText> floatTexts;

    std::mt19937                          rng;
    std::uniform_int_distribution<int>    colDist;
    std::uniform_real_distribution<float> ivDist;

    sf::Clock spawnClock;
    sf::Clock speedClock;
    float     nextSpawn = 0.f;

    bool  keyDown[3]   = {};
    float keyTimer[3]  = {};
    float missFlash[3] = {};
    float hitFlash[3]  = {};

    sf::RectangleShape rBlock, rTarget, rColBg;
    sf::RectangleShape rSep, rDivider, rPanel;
    sf::RectangleShape rOverlay, rNotifBg;

    sf::Text tScoreLabel, tScoreVal;
    sf::Text tCombo, tOver, tRestart, tFinal;
    sf::Text tWaiting, tCountdown;

    void restart();
    void onKeyPress(int col);
    void onLifeLost();
    void setupText(sf::Text& t, unsigned charSize, sf::Color color);
    void spawnFloatText(int col, int pts);

    void drawColumns(sf::RenderWindow& w);
    void drawBlocks(sf::RenderWindow& w);
    void drawHUD(sf::RenderWindow& w);
    void drawCountdown(sf::RenderWindow& w);
    void drawWaiting(sf::RenderWindow& w);
    void drawLifeNotif(sf::RenderWindow& w);

    static sf::SoundBuffer makePopSound();
    static sf::SoundBuffer makeLossSound();
};
