#pragma once
#include "common.h"
#include "system/joypad_state.h"
#include "system/interrupts.h"
#include "buttons.h"

class Joypad
{
private:
    Interrupts *interrupts;

    bool buttonSelected;
    bool dirSelected;
    JoypadState state;

    void setButtonState(Button button, bool isDown);
    // A button going down pulls one of the four input lines low, and it is
    // that high-to-low edge - not the press itself - that raises the joypad
    // interrupt.
    void raiseOnFallingEdge(u8 before);

public:
    Joypad(Interrupts *_interrupts);

    u8 read();
    void write(u8 value);

    void pressButton(Button button);
    void releaseButton(Button button);
};
