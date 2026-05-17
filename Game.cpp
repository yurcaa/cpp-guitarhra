#include "Game.h"
#include "Constants.h"
#include "Utils.h"
#include "HeartTexture.h"
#include <algorithm>
#include <cmath>
#include <sstream>
#include <vector>

using namespace sf;
using namespace std;

SoundBuffer Game::makePopSound()
{
    const int SR = 44100, N = SR * 55 / 1000;
    vector<Int16> s(N);
    for (int i = 0; i < N; ++i) {
        float t   = static_cast<float>(i) / SR;
        float env = exp(-t * 45.f);
        float frq = 900.f - 400.f * (t / 0.055f);
        s[i] = static_cast<Int16>(sin(2.f * 3.14159f * frq * t) * env * 28000.f);
    }
    SoundBuffer b; b.loadFromSamples(s.data(), N, 1, SR); return b;
}

SoundBuffer Game::makeLossSound()
{
    const int SR = 44100, N = SR * 500 / 1000;
    vector<Int16> s(N);
    for (int i = 0; i < N; ++i) {
        float t   = static_cast<float>(i) / SR;
        float env = exp(-t * 5.f);
        float frq = 350.f - 150.f * (t / 0.5f);
        float w   = sin(2.f * 3.14159f * frq * t) * 0.65f
                  + sin(2.f * 3.14159f * frq * 0.5f * t) * 0.35f;
        s[i] = static_cast<Int16>(w * env * 26000.f);
    }
    SoundBuffer b; b.loadFromSamples(s.data(), N, 1, SR); return b;
}

Game::Game(Font& f)
    : font(f)
    , rng(random_device{}())
    , colDist(0, COLS - 1)
    , ivDist(0.7f, 1.5f)
    , speed(SPEED_START)
    , rBlock  ({ COL_W,           BLOCK_H  })
    , rTarget ({ COL_W,           TARGET_H })
    , rColBg  ({ COL_W,           WIN_H    })
    , rSep    ({ SEP_W,           WIN_H    })
    , rDivider({ 2.f,             WIN_H    })
    , rPanel  ({ WIN_W - PANEL_X, WIN_H    })
    , rOverlay({ WIN_W,           WIN_H    })
    , rNotifBg({ 180.f,           68.f     })
{
    makeHeartTexture(heartTex);
    heartScale = 40.f / static_cast<float>(heartTex.getSize().x);
    heartSpr.setTexture(heartTex);
    heartSpr.setOrigin(heartTex.getSize().x / 2.f, heartTex.getSize().y / 2.f);

    popBuf  = makePopSound();  popSound.setBuffer(popBuf);   popSound.setVolume(25.f);
    lossBuf = makeLossSound(); lossSound.setBuffer(lossBuf); lossSound.setVolume(40.f);

    rBlock.setOutlineThickness(2.f);
    rTarget.setOutlineThickness(3.f);
    rSep.setFillColor(Color(0, 0, 0, 220));
    rDivider.setFillColor(Color(60, 60, 80, 200));
    rDivider.setPosition(PANEL_X - 2.f, 0.f);
    rPanel.setFillColor(Color(0, 0, 0, 50));
    rPanel.setPosition(PANEL_X, 0.f);
    rNotifBg.setFillColor(Color(10, 10, 20, 210));
    rNotifBg.setOutlineColor(Color(120, 120, 150, 180));
    rNotifBg.setOutlineThickness(2.f);
    rNotifBg.setOrigin(90.f, 0.f);

    setupText(tScoreLabel, 11, Color(180, 180, 200));
    setupText(tScoreVal,   20, Color::White);
    setupText(tCombo,      14, Color::Yellow);
    setupText(tOver,       28, Color::White);
    setupText(tRestart,    10, Color(200, 200, 200));
    setupText(tFinal,      14, Color::Yellow);
    setupText(tWaiting,    14, Color(220, 220, 255));
    setupText(tCountdown,  72, Color::White);

    tScoreLabel.setString("SCORE");
    tOver.setString("GAME OVER");
    tRestart.setString("PRESS R TO RESTART");
    tWaiting.setString("PRESS ANY KEY");

    nextSpawn = ivDist(rng);
}

void Game::handleEvent(const Event& event)
{
    if (event.type != Event::KeyPressed) return;
    const auto key = event.key.code;
    if (key == Keyboard::Escape || key == Keyboard::F11) return;

    switch (state)
    {
    case GameState::Waiting:
        state = GameState::Countdown;
        countdownTimer = 3.f;
        break;

    case GameState::Countdown:
        break;

    case GameState::Playing:
    {
        int col = -1;
        if (key == Keyboard::A) col = 0;
        if (key == Keyboard::S) col = 1;
        if (key == Keyboard::D) col = 2;
        if (col >= 0) onKeyPress(col);
        break;
    }

    case GameState::GameOver:
        if (key == Keyboard::R) restart();
        break;
    }
}

void Game::update(float dt)
{
    if (lostLifeTimer > 0.f) lostLifeTimer -= dt;

    switch (state)
    {
    case GameState::Countdown:
        countdownTimer -= dt;
        if (countdownTimer <= 0.f) {
            countdownTimer = 0.f;
            state = GameState::Playing;
            spawnClock.restart();
            speedClock.restart();
        }
        break;

    case GameState::Playing:
    {
        if (spawnClock.getElapsedTime().asSeconds() >= nextSpawn) {
            blocks.push_back({ colDist(rng), -BLOCK_H });
            spawnClock.restart();
            nextSpawn = ivDist(rng);
        }

        for (auto& b : blocks) {
            if (!b.active) { b.alpha -= dt * 600.f; continue; }
            b.y += speed * dt;
            if (b.y > WIN_H) {
                b.active = false; b.alpha = 0.f;
                missFlash[b.col] = 0.25f;
                combo = 0;
                onLifeLost();
            }
        }

        blocks.erase(
            remove_if(blocks.begin(), blocks.end(),
                [](const Block& b){ return !b.active && b.alpha <= 0.f; }),
            blocks.end());

        for (auto& ft : floatTexts) { ft.y += ft.vy * dt; ft.alpha -= dt * 280.f; }
        floatTexts.erase(
            remove_if(floatTexts.begin(), floatTexts.end(),
                [](const FloatText& ft){ return ft.alpha <= 0.f; }),
            floatTexts.end());

        if (speedClock.getElapsedTime().asSeconds() >= SPEED_EVERY) {
            speed = min(speed + SPEED_STEP, SPEED_MAX);
            speedClock.restart();
        }

        for (int i = 0; i < COLS; ++i) {
            if (keyTimer[i]  > 0.f) keyTimer[i]  -= dt; else keyDown[i]  = false;
            if (missFlash[i] > 0.f) missFlash[i] -= dt;
            if (hitFlash[i]  > 0.f) hitFlash[i]  -= dt;
        }
        break;
    }

    default: break;
    }
}

void Game::draw(RenderWindow& window)
{
    drawColumns(window);
    if (state == GameState::Playing || state == GameState::GameOver)
        drawBlocks(window);
    drawHUD(window);
    if (lostLifeTimer > 0.f)     drawLifeNotif(window);
    if (state == GameState::Waiting)   drawWaiting(window);
    if (state == GameState::Countdown) drawCountdown(window);

    if (state == GameState::GameOver)
    {
        rOverlay.setFillColor(Color(0, 0, 0, 170));
        window.draw(rOverlay);

        tOver.setPosition(WIN_W / 2.f - tOver.getLocalBounds().width / 2.f,
                          WIN_H / 2.f - 60.f);
        window.draw(tOver);

        tRestart.setPosition(WIN_W / 2.f - tRestart.getLocalBounds().width / 2.f,
                             WIN_H / 2.f + 10.f);
        window.draw(tRestart);

        ostringstream ss; ss << "SCORE: " << score;
        tFinal.setString(ss.str());
        tFinal.setPosition(WIN_W / 2.f - tFinal.getLocalBounds().width / 2.f,
                           WIN_H / 2.f + 40.f);
        window.draw(tFinal);
    }
}

void Game::drawColumns(RenderWindow& w)
{
    w.draw(rPanel);
    for (int i = 0; i < COLS; ++i) {
        rColBg.setFillColor(withAlpha(COL_COLOR[i], 38));
        rColBg.setPosition(COL_X[i], 0.f);
        w.draw(rColBg);
    }
    for (float sx : SEP_X) { rSep.setPosition(sx, 0.f); w.draw(rSep); }
    w.draw(rDivider);

    for (int i = 0; i < COLS; ++i) {
        Color tc = COL_COLOR[i];
        if      (missFlash[i] > 0.f) tc = Color(255, 60, 60);
        else if (hitFlash[i]  > 0.f) tc = Color::White;
        else if (keyDown[i])          tc = withAlpha(tc, 230);
        else                          tc = withAlpha(tc, 110);

        rTarget.setFillColor(tc);
        rTarget.setOutlineColor(Color(255, 255, 255, 160));
        rTarget.setPosition(COL_X[i], TARGET_Y - TARGET_H / 2.f);
        w.draw(rTarget);

        Text tKey("", font, 22);
        tKey.setFillColor(Color::White);
        tKey.setString(string(1, COL_KEY[i]));
        tKey.setPosition(COL_X[i] + COL_W / 2.f - tKey.getLocalBounds().width / 2.f,
                         TARGET_Y - 14.f);
        w.draw(tKey);
    }
}

void Game::drawBlocks(RenderWindow& w)
{
    for (auto& b : blocks) {
        Color fill; Uint8 a;
        if (b.active) {
            fill = COL_COLOR[b.col]; a = 230;
        } else if (b.hitFlash) {
            a = static_cast<Uint8>(max(0.f, b.alpha));
            fill = Color(255, 255, 255, a);
        } else {
            a = static_cast<Uint8>(max(0.f, b.alpha));
            fill = withAlpha(COL_COLOR[b.col], a);
        }
        rBlock.setFillColor(fill);
        rBlock.setOutlineColor(Color(255, 255, 255, b.active ? 80 : a / 3));
        rBlock.setPosition(COL_X[b.col], b.y);
        w.draw(rBlock);
    }

    for (auto& ft : floatTexts) {
        Text t(ft.text, font, 16);
        t.setFillColor(Color(255, 255, 180, static_cast<Uint8>(max(0.f, ft.alpha))));
        t.setPosition(ft.x - t.getLocalBounds().width / 2.f, ft.y);
        w.draw(t);
    }
}

void Game::drawHUD(RenderWindow& w)
{
    tScoreLabel.setPosition(PANEL_CX - tScoreLabel.getLocalBounds().width / 2.f, 25.f);
    w.draw(tScoreLabel);

    ostringstream ss; ss << score;
    tScoreVal.setString(ss.str());
    tScoreVal.setPosition(PANEL_CX - tScoreVal.getLocalBounds().width / 2.f, 45.f);
    w.draw(tScoreVal);

    if (combo >= 3) {
        ss.str(""); ss << "COMBO\nx" << combo;
        tCombo.setString(ss.str());
        tCombo.setPosition(PANEL_CX - tCombo.getLocalBounds().width / 2.f,
                           WIN_H / 2.f - 40.f);
        w.draw(tCombo);
    }
}

void Game::drawLifeNotif(RenderWindow& w)
{
    const float TOTAL = 1.8f, t = lostLifeTimer;
    float slide;
    if      (t > TOTAL - 0.28f) slide = (TOTAL - t) / 0.28f;
    else if (t < 0.28f)         slide = t / 0.28f;
    else                        slide = 1.f;
    slide = slide * slide * (3.f - 2.f * slide);   // cubic ease

    const float notifY = -75.f + 85.f * slide;
    const Uint8 alpha  = static_cast<Uint8>(slide * 230.f);
    const float cx     = (COL_X[0] + COL_X[COLS - 1] + COL_W) / 2.f;

    rNotifBg.setFillColor(Color(10, 10, 20, alpha));
    rNotifBg.setOutlineColor(Color(120, 120, 150, alpha));
    rNotifBg.setPosition(cx, notifY);
    w.draw(rNotifBg);

    float sc      = heartScale * 0.85f;
    float hw      = heartTex.getSize().x * sc;
    float spacing = hw + 8.f;
    float startX  = cx - (lostLifeCount * spacing - 8.f) / 2.f + hw / 2.f;

    for (int i = 0; i < lostLifeCount; ++i) {
        heartSpr.setScale(sc, sc);
        heartSpr.setColor(Color(255, 255, 255, alpha));
        heartSpr.setPosition(startX + i * spacing, notifY + 34.f);
        w.draw(heartSpr);
    }
    heartSpr.setColor(Color::White);
}

void Game::drawCountdown(RenderWindow& w)
{
    int digit = static_cast<int>(ceil(countdownTimer));
    if (digit < 1) return;

    float frac  = countdownTimer - floor(countdownTimer);
    float scale = 1.f + 1.8f * max(0.f, 1.f - frac / 0.35f);
    float alpha = frac < 0.72f ? 255.f : (1.f - (frac - 0.72f) / 0.28f) * 255.f;

    Color col = (digit == 3) ? Color::White
              : (digit == 2) ? Color::Yellow
                             : Color(255, 100, 100);
    col.a = static_cast<Uint8>(alpha);

    tCountdown.setString(to_string(digit));
    tCountdown.setFillColor(col);

    FloatRect b = tCountdown.getLocalBounds();
    tCountdown.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f);
    tCountdown.setScale(scale, scale);
    tCountdown.setPosition((COL_X[0] + COL_X[COLS - 1] + COL_W) / 2.f, WIN_H / 2.f - 40.f);
    w.draw(tCountdown);
    tCountdown.setScale(1.f, 1.f);
}

void Game::drawWaiting(RenderWindow& w)
{
    rOverlay.setFillColor(Color(0, 0, 0, 140));
    w.draw(rOverlay);

    static Clock waitClock;
    float pulse = 0.55f + 0.45f * sin(waitClock.getElapsedTime().asSeconds() * 2.5f);
    tWaiting.setFillColor(Color(220, 220, 255, static_cast<Uint8>(pulse * 255.f)));
    tWaiting.setPosition(WIN_W / 2.f - tWaiting.getLocalBounds().width / 2.f,
                         WIN_H / 2.f - 10.f);
    w.draw(tWaiting);
}

void Game::restart()
{
    lives = 3; score = 0; combo = 0;
    speed = SPEED_START; lostLifeTimer = 0.f;
    blocks.clear(); floatTexts.clear();
    state = GameState::Countdown; countdownTimer = 3.f;
}

void Game::onLifeLost()
{
    if (--lives <= 0) { lives = 0; state = GameState::GameOver; }
    lossSound.play();
    lostLifeTimer = 1.8f;
    lostLifeCount = lives;
}

void Game::onKeyPress(int col)
{
    keyDown[col] = true; keyTimer[col] = 0.12f;

    Block* best = nullptr; float bestDist = HIT_WIN + 1.f;
    for (auto& b : blocks) {
        if (!b.active || b.col != col) continue;
        float d = abs(b.y + BLOCK_H / 2.f - TARGET_Y);
        if (d < bestDist) { bestDist = d; best = &b; }
    }

    if (best) {
        best->active = false; best->hitFlash = true; best->alpha = 255.f;
        combo++;
        int pts = 100 + (combo - 1) * 15;
        score += pts;
        hitFlash[col] = 0.12f;
        popSound.play();
        spawnFloatText(col, pts);
    } else {
        combo = 0;
    }
}

void Game::spawnFloatText(int col, int pts)
{
    floatTexts.push_back({
        "+" + to_string(pts),
        COL_X[col] + COL_W / 2.f,
        TARGET_Y - 50.f, 255.f, -80.f
    });
}

void Game::setupText(Text& t, unsigned sz, Color c)
{
    t.setFont(font); t.setCharacterSize(sz); t.setFillColor(c);
}
