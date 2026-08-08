#pragma once

enum InterruptType : u8
{
    VBlank = 1,
    LCD_ = 2,
    Timer_ = 4,
    Serial = 8,
    Joypad_ = 16
};