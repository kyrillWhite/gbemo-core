#include "memory/memory_io.h"

MemoryIO::MemoryIO(
    Interrupts *_interrupts,
    Timer *_timer,
    LCD *_lcd,
    APU *_apu,
    Joypad *_joypad) : interrupts(_interrupts),
                       timer(_timer),
                       booted(false),
                       joypad(_joypad),
                       lcd(_lcd),
                       apu(_apu),
                       serialLog{},
                       serialLogSize(0)
{
    serial[0] = 0x00;
    serial[1] = 0x7E;
}

bool MemoryIO::isBooted()
{
    return booted;
}

void MemoryIO::transferSerial()
{
    if (serialLogSize < SERIAL_LOG_CAPACITY - 1)
    {
        serialLog[serialLogSize++] = static_cast<char>(serial[0]);
        serialLog[serialLogSize] = '\0';
    }

    serial[0] = 0xFF;
    serial[1] &= ~0x80;
    interrupts->setIFflag(Serial, true);
}

const char *MemoryIO::getSerialLog() const
{
    return serialLog;
}

u32 MemoryIO::getSerialLogSize() const
{
    return serialLogSize;
}

u8 MemoryIO::read(u16 from)
{
    // joypad input
    if (from == 0xFF00)
    {
        return joypad->read();
    }
    // serial transfer
    else if (from >= 0xFF01 && from <= 0xFF02)
    {
        return from == 0xFF02 ? (serial[1] | 0x7E) : serial[0];
    }
    // timer and divider
    else if (from >= 0xFF04 && from <= 0xFF07)
    {
        return timer->read(from);
    }
    // interrupts flag (IF) - the three unused bits read as one
    else if (from == 0xFF0F)
    {
        return interrupts->getIF() | 0xE0;
    }
    // audio
    else if (from >= 0xFF10 && from <= 0xFF26)
    {
        return apu->read(from);
    }
    // wave pattern
    else if (from >= 0xFF30 && from <= 0xFF3F)
    {
        return apu->read(from);
    }
    // lcd
    else if (from >= 0xFF40 && from <= 0xFF4B)
    {
        return static_cast<u8>(lcd->read(from));
    }
    // set to non-zero to disable boot rom
    else if (from == 0xFF50)
    {
        return 0xFF;
    }
    else
    {
        // an unmapped I/O register floats high
        return 0xFF;
    }
}

void MemoryIO::write(u16 to, u8 value)
{
    // joypad input
    if (to == 0xFF00)
    {
        joypad->write(value);
    }
    // serial transfer
    else if (to == 0xFF01)
    {
        serial[0] = value;
    }
    else if (to == 0xFF02)
    {
        serial[1] = value;
        // Bit 0 picks the internal clock: only then does this side drive the
        // transfer, and only then does it finish on its own.
        if ((value & 0x81) == 0x81)
        {
            transferSerial();
        }
    }
    // timer and divider
    else if (to >= 0xFF04 && to <= 0xFF07)
    {
        timer->write(to, value);
    }
    // interrupts flag (IF)
    else if (to == 0xFF0F)
    {
        interrupts->setIF(value & 0x1F);
    }
    // audio
    else if (to >= 0xFF10 && to <= 0xFF26)
    {
        apu->write(to, value);
    }
    // wave pattern
    else if (to >= 0xFF30 && to <= 0xFF3F)
    {
        apu->write(to, value);
    }
    // lcd
    else if (to >= 0xFF40 && to <= 0xFF4B)
    {
        lcd->write(to, value);
    }
    // set to non-zero to disable boot rom
    else if (to == 0xFF50)
    {
        booted = value ? true : booted;
    }
    else
    {
        return;
    }
}
