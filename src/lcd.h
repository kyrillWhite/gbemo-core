#pragma once
#include "lcd_registers.h"
#include "ppu_mode.h"
#include "dma.h"


class LCD
{
private:
    LcdRegisters* registers;
    DMA* dma;

public:
    LCD(DMA* _dma, bool skipBoot = false);
    ~LCD();

    u16 read(u16 address);
    void write(u16 address, u8 value);
    void updatePalette(u8 paletteData, u8 pal);

    LcdRegisters* getRegisters();

    //LCDC
    bool bgwEnable();
    bool objEnable();
    u8 objSize();
    u16 bgTileMapArea();
    u16 bgAndWinTileDataArea();
    bool windowEnable();
    u16 winTileMapArea();
    bool enable();

    //LCDS
    PpuMode mode();
    void modeSet(PpuMode _mode);
    bool lyc();
    void lycSet(bool _value);
    bool statInt(StatSrc stat);
};

