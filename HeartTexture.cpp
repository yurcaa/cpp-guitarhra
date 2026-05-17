#include "HeartTexture.h"

using namespace sf;

void makeHeartTexture(Texture& tex)
{
    static const int MAP[11][13] = {
        {0,0,1,1,0,0,0,1,1,0,0,0,0},
        {0,1,1,1,1,0,1,1,1,1,0,0,0},
        {1,1,2,2,1,1,1,1,1,1,1,0,0},
        {1,2,2,1,1,1,1,1,1,1,1,1,0},
        {1,1,1,1,1,1,1,1,1,1,1,1,0},
        {0,1,1,1,1,1,1,1,1,1,1,1,0},
        {0,1,1,1,1,1,1,1,1,1,1,0,0},
        {0,0,1,1,1,1,1,1,1,1,0,0,0},
        {0,0,0,1,1,1,1,1,1,0,0,0,0},
        {0,0,0,0,1,1,1,1,0,0,0,0,0},
        {0,0,0,0,0,1,1,0,0,0,0,0,0},
    };

    constexpr int HW = 13, HH = 11, PS = 8;
    constexpr int TW = HW * PS + 2, TH = HH * PS + 2;

    Image img;
    img.create(TW, TH, Color::Transparent);

    for (int r = 0; r < HH; ++r)
        for (int c = 0; c < HW; ++c) {
            if (!MAP[r][c]) continue;
            int px = c * PS + 1, py = r * PS + 1;
            for (int dy = -1; dy <= PS; ++dy)
                for (int dx = -1; dx <= PS; ++dx) {
                    int nx = px + dx, ny = py + dy;
                    if (nx < 0 || ny < 0 || nx >= TW || ny >= TH) continue;
                    if (img.getPixel(nx, ny).a == 0)
                        img.setPixel(nx, ny, Color::Black);
                }
        }

    for (int r = 0; r < HH; ++r)
        for (int c = 0; c < HW; ++c) {
            if (!MAP[r][c]) continue;
            Color fill = (MAP[r][c] == 2) ? Color::White : Color(210, 0, 0);
            int px = c * PS + 1, py = r * PS + 1;
            for (int dy = 0; dy < PS; ++dy)
                for (int dx = 0; dx < PS; ++dx)
                    img.setPixel(px + dx, py + dy, fill);
        }

    tex.loadFromImage(img);
}
