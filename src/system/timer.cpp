#include "system/timer.h"

// which bit of the 16-bit divider clocks TIMA, per TAC's low two bits
static constexpr u8 CLOCK_BIT[4] = {9, 3, 5, 7};

Timer::Timer(Interrupts *_interrupts, APU *_apu, bool skipBoot) : interrupts(_interrupts),
                                                                  apu(_apu),
                                                                  div(0), tima(0), tma(0), tac(0),
                                                                  reloadDelay(0),
                                                                  reloadedThisTick(false),
                                                                  prevEdge(false)
{
    if (skipBoot)
    {
        div = 0xABCC;
    }
}

void Timer::updateEdge()
{
    bool edge = bit(tac, 2) && ((div >> CLOCK_BIT[tac & 0b11]) & 1);

    if (prevEdge && !edge)
    {
        tima++;
        if (tima == 0x00)
        {
            reloadDelay = 4;
        }
    }

    prevEdge = edge;
}

void Timer::tick()
{
    reloadedThisTick = false;

    if (reloadDelay > 0 && --reloadDelay == 0)
    {
        tima = tma;
        interrupts->setIFflag(InterruptType::Timer_, true);
        reloadedThisTick = true;
    }

    u16 prevDiv = div;
    div++;

    if ((prevDiv & (1 << 4)) && (!(div & (1 << 4))))
    {
        apu->increaseDiv();
    }

    updateEdge();
}

void Timer::write(u16 address, u8 value)
{
    switch (address)
    {
    case 0xFF04:
        // Resetting the divider drops the clock bit with it, and that falling
        // edge ticks TIMA just as an ordinary one would.
        div = 0;
        updateEdge();
        break;
    case 0xFF05:
        // On the very cycle the reload lands, the write is swallowed.
        if (reloadedThisTick)
        {
            break;
        }
        // Anywhere in the four cycles before it, the write cancels both the
        // reload and the interrupt it would have raised.
        reloadDelay = 0;
        tima = value;
        break;
    case 0xFF06:
        tma = value;
        // Written on the reload cycle, the new TMA is what TIMA gets.
        if (reloadedThisTick)
        {
            tima = value;
        }
        break;
    case 0xFF07:
        tac = value & 0b111;
        // Changing the selected bit, or clearing the enable while that bit is
        // high, is another way to produce a falling edge.
        updateEdge();
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
        // only three bits are wired; the rest read as one
        return tac | 0xF8;
    default:
        NOT_AVAILABLE;
    }
}
