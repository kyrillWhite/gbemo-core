#include "memory_io.h"

MemoryIO::MemoryIO(
    Interrupts* _interrupts,
    Timer* _timer,
    LCD* _lcd,
    Joypad* _joypad,
    APU* _apu
) :
    interrupts(_interrupts),
    timer(_timer),
    booted(false),
    serial(new u8[2]),
    joypad(_joypad),
    lcd(_lcd),
    apu(_apu)
{
    serial[0] = 0x00;
    serial[1] = 0x7E;
}

MemoryIO::~MemoryIO()
{
    // lcd, interrupts, timer, joypad and apu are owned by Emu, not by us.
    delete[] serial;
}

bool MemoryIO::isBooted()
{
    return booted;
}

u8 MemoryIO::read(u16 from)
{
    // joypad input
    if (from == 0xFF00) { 
        return joypad->read();
    }
    // serial transfer
    else if (from >= 0xFF01 && from <= 0xFF02) {
        return serial[from - 0xFF01];
    }
    // timer and divider
    else if (from >= 0xFF04 && from <= 0xFF07) {
        return timer->read(from);
    }
    // interrupts flag (IF)
    else if (from == 0xFF0F) { 
        return interrupts->getIF();
    }
    // audio
    else if (from >= 0xFF10 && from <= 0xFF26) { 
        return apu->read(from);
    }
    // wave pattern
    else if (from >= 0xFF30 && from <= 0xFF3F) { 
        return apu->read(from);
    }
    // lcd
    else if (from >= 0xFF40 && from <= 0xFF4B) {
        return lcd->read(from);
    }
    // set to non-zero to disable boot rom
    else if (from == 0xFF50) {
        return 0;
        NOT_AVAILABLE
    }
    else {
        return 0;
        NOT_AVAILABLE
    }
}

void MemoryIO::write(u16 to, u8 value)
{
    // joypad input
    if (to == 0xFF00) {
        joypad->write(value);
    }
    // serial transfer
    else if (to >= 0xFF01 && to <= 0xFF02) {
        serial[to - 0xFF01] = value;
    }
    // timer and divider
    else if (to >= 0xFF04 && to <= 0xFF07) {
        timer->write(to, value);
    }
    // interrupts flag (IF)
    else if (to == 0xFF0F) { 
        interrupts->setIF(value);
    }
    // audio
    else if (to >= 0xFF10 && to <= 0xFF26) { 
        apu->write(to, value);
    }
    // wave pattern
    else if (to >= 0xFF30 && to <= 0xFF3F) { 
        apu->write(to, value);
    }
    // lcd
    else if (to >= 0xFF40 && to <= 0xFF4B) {
        lcd->write(to, value);
    }
    // set to non-zero to disable boot rom
    else if (to == 0xFF50) { 
        booted = value ? true : booted;
    }
    else {
        return;
        NOT_AVAILABLE
    }
}
