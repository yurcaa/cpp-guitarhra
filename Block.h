#pragma once

struct Block
{
    int   col;
    float y;
    bool  active   = true;
    float alpha    = 255.f;
    bool  hitFlash = false;
};
