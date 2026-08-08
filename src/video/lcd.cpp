#include "video/lcd.h"

LCD::LCD(DMA *_dma, bool skipBoot) : dma(_dma)
{
    if (skipBoot)
    {
        registers.lcdc = 0x91;
        registers.scrollX = 0;
        registers.scrollY = 0;
        registers.ly = 0;
        registers.lyCompare = 0;
        // registers.dma = 0xFF;
        registers.bgPalette = 0xFC;
        registers.objPalette[0] = 0xFF;
        registers.objPalette[1] = 0xFF;
        registers.windowX = 0;
        registers.windowY = 0;

        for (int i = 0; i < 4; i++)
        {
            registers.bgColors[i] = i;
            registers.sp1Colors[i] = i;
            registers.sp2Colors[i] = i;
        }
    }
}

u16 LCD::read(u16 address)
{
    u8 offset = (address - 0xFF40);
    u8 *p = (u8 *)&registers;
    return p[offset];
}

void LCD::write(u16 address, u8 value)
{
    u8 offset = (address - 0xFF40);
    u8 *p = (u8 *)&registers;
    p[offset] = value;

    if (offset == 6)
    {
        dma->start(value);
    }

    if (address == 0xFF47)
    {
        updatePalette(value, 0);
    }
    else if (address == 0xFF48)
    {
        updatePalette(value & 0b11111100, 1);
    }
    else if (address == 0xFF49)
    {
        updatePalette(value & 0b11111100, 2);
    }
}

void LCD::updatePalette(u8 paletteData, u8 pal)
{
    u8 *pColors = registers.bgColors;

    switch (pal)
    {
    case 1:
        pColors = registers.sp1Colors;
        break;
    case 2:
        pColors = registers.sp2Colors;
        break;
    }

    pColors[0] = paletteData & 0b11;
    pColors[1] = (paletteData >> 2) & 0b11;
    pColors[2] = (paletteData >> 4) & 0b11;
    pColors[3] = (paletteData >> 6) & 0b11;
}

LcdRegisters *LCD::getRegisters()
{
    return &registers;
}

bool LCD::bgwEnable()
{
    return bit(registers.lcdc, 0);
}

bool LCD::objEnable()
{
    return bit(registers.lcdc, 1);
}

u8 LCD::objSize()
{
    return bit(registers.lcdc, 2) ? 16 : 8;
}

u16 LCD::bgTileMapArea()
{
    return bit(registers.lcdc, 3) ? 0x9C00 : 0x9800;
}

u16 LCD::bgAndWinTileDataArea()
{
    return bit(registers.lcdc, 4) ? 0x8000 : 0x8800;
}

bool LCD::windowEnable()
{
    return bit(registers.lcdc, 5);
}

u16 LCD::winTileMapArea()
{
    return bit(registers.lcdc, 6) ? 0x9C00 : 0x9800;
}

bool LCD::enable()
{
    return bit(registers.lcdc, 7);
}

PpuMode LCD::mode()
{
    return PpuMode(registers.lcds & 0b11);
}

void LCD::modeSet(PpuMode _mode)
{
    registers.lcds &= ~0b11;
    registers.lcds |= _mode;
}

bool LCD::lyc()
{
    return bit(registers.lcds, 2);
}

void LCD::lycSet(bool _value)
{
    registers.lcds = set_bit(registers.lcds, _value, 2);
}

bool LCD::statInt(StatSrc stat)
{
    return registers.lcds & stat;
}
