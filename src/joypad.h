#pragma once
#include "common.h"
#include "joypad_state.h"
#include "buttons.h"

class Joypad
{
private:
    bool buttonSelected;
    bool dirSelected;
    JoypadState state;

    void setButtonState(Button button, bool isDown);

public:
    Joypad();

    u8 read();
    void write(u8 value);

    void pressButton(Button button);
    void releaseButton(Button button);
};
