#pragma once
#include "memory/oam.h"
#include "mono_color.h"
#include "video/lcd.h"
#include "video/ppu_sm.h"
#include "video/fifo.h"

constexpr int LINES_PER_FRAME = 154;
constexpr int TICKS_PER_LINE = 456;
constexpr int YRES = 144;
constexpr int XRES = 160;

// A scanline may select ten sprites, and all ten can overlap the same
// eight-pixel fetch window - a smaller array here silently drops whichever
// ones come last, which on DMG are the ones furthest right.
constexpr int MAX_LINE_SPRITES = 10;

class PPU
{
private:
    LCD *lcd;
    PpuStateMachine *ppuSm;
    FIFO fifo;

    OAMEntry oam[40]; // TODO: move to ppu
    u8 vram[8192];

    u8 videoBuffer[YRES * XRES];

    bool lcdWasEnabled;

public:
    u32 lineTicks;
    u32 currentFrame;

    PPU(LCD *_lcd, bool skipBoot);

    u8 lineSpriteCount;
    OAMLineEntry *lineSprites;

    u8 fetchedEntryCount;
    OAMEntry fetchedEntries[MAX_LINE_SPRITES];
    OAMLineEntry lineEntryArray[MAX_LINE_SPRITES];
    u8 windowLine;

    void tick();

    bool windowVisible();
    void loadLineSpites();

    void setStateMachine(PpuStateMachine *_ppuSm);

    u8 oamRead(u16 address, bool inner = false);
    void oamWrite(u16 address, u8 value, bool inner = false);
    u8 vramRead(u16 address, bool inner = false);
    void vramWrite(u16 address, u8 value, bool inner = false);

    FIFO *getFIFO();

    u8 fetchSpritePixels(u8 color, u8 bgColor);
    void pipelineLoadWindowTile();
    void pipelineLoadSpriteData(u8 offset);
    void pipelineLoadSpriteTile();
    void pipelineFetch();
    void pipelinePushPixel();
    void pipelineProcess();
    bool pipelineFifoAdd();

    u8 *getVideoBuffer();

    // debug
    u8 *getVRAM();
};
