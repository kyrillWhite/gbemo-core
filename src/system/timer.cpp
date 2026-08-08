#include "system/timer.h"

Timer::Timer(Interrupts *_interrupts, APU *_apu, bool skipBoot) : interrupts(_interrupts),
                                                                  apu(_apu),
                                                                  div(0), tima(0), tma(0), tac(0)
{
    if (skipBoot)
    {
        div = 0xABCC;
    }
}

void Timer::tick()
{
    u16 prevDiv = div;
    div++;

    if ((prevDiv & (1 << 4)) && (!(div & (1 << 4))))
    {
        apu->increaseDiv();
    }

    if (bit(tac, 2))
    {
        int every = 0;
        bool timer_update = false;
        switch (tac & 0b11)
        {
        case 0b00:
            timer_update = (prevDiv & (1 << 9)) && (!(div & (1 << 9)));
            break;
        case 0b01:
            timer_update = (prevDiv & (1 << 3)) && (!(div & (1 << 3)));
            break;
        case 0b10:
            timer_update = (prevDiv & (1 << 5)) && (!(div & (1 << 5)));
            break;
        case 0b11:
            timer_update = (prevDiv & (1 << 7)) && (!(div & (1 << 7)));
            break;
        }
        if (timer_update)
        {
            tima++;

            if (tima == 0x00)
            {
                tima = tma;
                interrupts->setIFflag(InterruptType::Timer_, true);
            }
        }
    }
}

void Timer::write(u16 address, u8 value)
{
    switch (address)
    {
    case 0xFF04:
        div = 0;
        break;
    case 0xFF05:
        tima = value;
        break;
    case 0xFF06:
        tma = value;
        break;
    case 0xFF07:
        tac = value;
        break;
    default:
        NOT_AVAILABLE;
    }
}

u8 Timer::read(u16 address)
{
    switch (address)
    {
    case 0xFF04:
        return div >> 8;
    case 0xFF05:
        return tima;
    case 0xFF06:
        return tma;
    case 0xFF07:
        return tac;
    default:
        NOT_AVAILABLE;
    }
}
