#pragma once
#include "common.h"

struct OAMEntry
{
    u8 yPos;
    u8 xPos;
    u8 tile;

    u8 cgbPalette : 3;
    u8 bank : 1;
    u8 dmgPalette : 1;
    u8 xFlip : 1;
    u8 yFlip : 1;
    u8 priority : 1;
};

struct OAMLineEntry
{
    OAMEntry oam;
    OAMLineEntry *next;
};