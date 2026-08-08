#pragma once
#include "common.h"

struct LcdRegisters
{
    // registers...
    u8 lcdc;
    u8 lcds;
    u8 scrollY;
    u8 scrollX;
    u8 ly;
    u8 lyCompare;
    u8 dma;
    u8 bgPalette;
    u8 objPalette[2];
    u8 windowY;
    u8 windowX;

    // other data...
    u8 bgColors[4];
    u8 sp1Colors[4];
    u8 sp2Colors[4];
};