#include "system/joypad.h"

void Joypad::setButtonState(Button button, bool isDown)
{
    switch (button)
    {
    case B_START:
        state.start = isDown;
        break;
    case B_SELECT:
        state.select = isDown;
        break;
    case B_A:
        state.a = isDown;
        break;
    case B_B:
        state.b = isDown;
        break;
    case B_UP:
        state.up = isDown;
        break;
    case B_DOWN:
        state.down = isDown;
        break;
    case B_LEFT:
        state.left = isDown;
        break;
    case B_RIGHT:
        state.right = isDown;
        break;
    default:
        break;
    }
}

Joypad::Joypad(Interrupts *_interrupts) : interrupts(_interrupts),
                                          state{0, 0, 0, 0, 0, 0, 0, 0},
                                          buttonSelected(false),
                                          dirSelected(false)
{
}

void Joypad::raiseOnFallingEdge(u8 before)
{
    u8 after = read();
    if ((before & ~after) & 0x0F)
    {
        interrupts->setIFflag(Joypad_, true);
    }
}

u8 Joypad::read()
{
    u8 output = 0xC0;
    output |= buttonSelected ? 0x20 : 0;
    output |= dirSelected ? 0x10 : 0;
    output |= 0x0F;

    if (!buttonSelected)
    {
        if (state.start)
        {
            output &= ~(1 << 3);
        }
        if (state.select)
        {
            output &= ~(1 << 2);
        }
        if (state.a)
        {
            output &= ~(1 << 0);
        }
        if (state.b)
        {
            output &= ~(1 << 1);
        }
    }

    if (!dirSelected)
    {
        if (state.left)
        {
            output &= ~(1 << 1);
        }
        if (state.right)
        {
            output &= ~(1 << 0);
        }
        if (state.up)
        {
            output &= ~(1 << 2);
        }
        if (state.down)
        {
            output &= ~(1 << 3);
        }
    }

    return output;
}

void Joypad::write(u8 value)
{
    u8 before = read();

    // Both select lines are active low.
    buttonSelected = value & 0x20;
    dirSelected = value & 0x10;

    raiseOnFallingEdge(before);
}

void Joypad::pressButton(Button button)
{
    u8 before = read();
    setButtonState(button, true);
    raiseOnFallingEdge(before);
}

void Joypad::releaseButton(Button button)
{
    setButtonState(button, false);
}
