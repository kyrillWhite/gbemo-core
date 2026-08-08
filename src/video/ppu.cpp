#include "video/ppu.h"
#include <cstring>

PPU::PPU(LCD *_lcd, bool skipBoot) : lcd(_lcd),
                                     currentFrame(0),
                                     lineTicks(0),
                                     lineSpriteCount(0),
                                     lineSprites(nullptr),
                                     fetchedEntryCount(0),
                                     windowLine(0)
{
    lcd->modeSet(OAM);
    memset(videoBuffer, 0, YRES * XRES);
    memset(vram, 0, 8192);

    if (skipBoot)
    {
        memset(oam, 0, sizeof(OAMEntry) * 40);
    }
}

void PPU::setStateMachine(PpuStateMachine *_ppuSm)
{
    ppuSm = _ppuSm;
}

void PPU::tick()
{
    lineTicks++;

    switch (lcd->mode())
    {
    case HBLANK:
        ppuSm->hblankMode();
        break;
    case VBLANK:
        ppuSm->vblankMode();
        break;
    case OAM:
        ppuSm->oamMode();
        break;
    case XFER:
        ppuSm->xferMode();
        break;
    }
}

bool PPU::windowVisible()
{
    return lcd->windowEnable() && lcd->getRegisters()->windowX >= 0 &&
           lcd->getRegisters()->windowX <= 166 && lcd->getRegisters()->windowY >= 0 &&
           lcd->getRegisters()->windowY < YRES;
}

void PPU::loadLineSpites()
{
    lineSpriteCount = 0;
    lineSprites = nullptr;

    int curY = lcd->getRegisters()->ly;
    u8 sprite_height = lcd->objSize();

    for (int i = 0; i < 40; i++)
    {
        OAMEntry e = oam[i];
        if (!e.xPos)
            continue;
        if (lineSpriteCount >= 10)
            break;

        if (e.yPos <= curY + 16 && e.yPos + sprite_height > curY + 16)
        {
            auto *entry = &lineEntryArray[lineSpriteCount++];
            entry->oam = e;
            entry->next = nullptr;

            if (!lineSprites || e.xPos < lineSprites->oam.xPos)
            {
                entry->next = lineSprites;
                lineSprites = entry;
            }
            else
            {
                OAMLineEntry *prev = lineSprites;
                OAMLineEntry *cur = lineSprites->next;
                while (cur)
                {
                    if (e.xPos <= cur->oam.xPos)
                    {
                        prev->next = entry;
                        entry->next = cur;
                        break;
                    }
                    prev = cur;
                    cur = cur->next;
                }
                if (!cur)
                {
                    prev->next = entry;
                }
            }
        }
    }
}

u8 PPU::oamRead(u16 address, bool inner)
{
    if (address >= 0xFE00)
    {
        address -= 0xFE00;
    }

    u8 *p = (u8 *)oam;
    return p[address];
}

void PPU::oamWrite(u16 address, u8 value, bool inner)
{
    if (address >= 0xFE00)
    {
        address -= 0xFE00;
    }

    u8 *p = (u8 *)oam;
    p[address] = value;
}

u8 PPU::vramRead(u16 address, bool inner)
{
    return vram[address - 0x8000];
}

void PPU::vramWrite(u16 address, u8 value, bool inner)
{
    vram[address - 0x8000] = value;
}

u32 PPU::getLineTicks()
{
    return lineTicks;
}

void PPU::setLineTicks(u32 value)
{
    lineTicks = value;
}

u32 PPU::getCurrentFrame()
{
    return currentFrame;
}

void PPU::setCurrentFrame(u32 value)
{
    currentFrame = value;
}

FIFO *PPU::getFIFO()
{
    return &fifo;
}

u8 PPU::fetchSpritePixels(int bit, u8 color, u8 bgColor)
{
    for (int i = 0; i < fetchedEntryCount; i++)
    {
        int spX = (fetchedEntries[i].xPos - 8) +
                  ((lcd->getRegisters()->scrollX % 8));

        if (spX + 8 < fifo.fifoX)
        {
            continue;
        }

        int offset = fifo.fifoX - spX;

        if (offset < 0 || offset > 7)
        {
            continue;
        }

        bit = (7 - offset);

        if (fetchedEntries[i].xFlip)
        {
            bit = offset;
        }

        u8 hi = !!(fifo.fetchEntryData[i * 2] & (1 << bit));
        u8 lo = !!(fifo.fetchEntryData[(i * 2) + 1] & (1 << bit)) << 1;

        bool bgPriority = fetchedEntries[i].priority;

        if (!(hi | lo))
        {
            continue;
        }

        if (!bgPriority || bgColor == 0)
        {
            color = (fetchedEntries[i].dmgPalette) ? lcd->getRegisters()->sp2Colors[hi | lo] : lcd->getRegisters()->sp1Colors[hi | lo];

            if (hi | lo)
            {
                break;
            }
        }
    }

    return color;
}

void PPU::pipelineLoadWindowTile()
{
    if (!windowVisible())
    {
        return;
    }

    u8 window_y = lcd->getRegisters()->windowY;

    if (fifo.fetchX + 7 >= lcd->getRegisters()->windowX &&
        fifo.fetchX + 7 < lcd->getRegisters()->windowX + YRES + 14)
    {
        if (lcd->getRegisters()->ly >= window_y && lcd->getRegisters()->ly < window_y + XRES)
        {
            u8 w_tile_y = windowLine / 8;

            fifo.bgwFetchData[0] = vramRead(lcd->winTileMapArea() +
                                                ((fifo.fetchX + 7 - lcd->getRegisters()->windowX) / 8) +
                                                (w_tile_y * 32),
                                            true);

            if (lcd->bgAndWinTileDataArea() == 0x8800)
            {
                fifo.bgwFetchData[0] += 128;
            }
        }
    }
}

void PPU::pipelineLoadSpriteData(u8 offset)
{
    int curY = lcd->getRegisters()->ly;
    u8 spriteHeight = lcd->objSize();

    for (int i = 0; i < fetchedEntryCount; i++)
    {
        u8 ty = ((curY + 16) - fetchedEntries[i].yPos) * 2;

        if (fetchedEntries[i].yFlip)
        {
            ty = ((spriteHeight * 2) - 2) - ty;
        }

        u8 tile_index = fetchedEntries[i].tile;

        if (spriteHeight == 16)
        {
            tile_index &= ~(1);
        }

        fifo.fetchEntryData[(i * 2) + offset] =
            vramRead(0x8000 + (tile_index * 16) + ty + offset, true);
    }
}

void PPU::pipelineLoadSpriteTile()
{
    OAMLineEntry *le = lineSprites;

    while (le)
    {
        int spX = (le->oam.xPos - 8) + (lcd->getRegisters()->scrollX % 8);

        if ((spX >= fifo.fetchX && spX < fifo.fetchX + 8) ||
            ((spX + 8) >= fifo.fetchX && (spX + 8) < fifo.fetchX + 8))
        {
            fetchedEntries[fetchedEntryCount++] = le->oam;
        }

        le = le->next;

        if (!le || fetchedEntryCount >= 3)
        {
            break;
        }
    }
}

void PPU::pipelineFetch()
{
    switch (fifo.fetchState)
    {
    case FS_TILE:
    {
        fetchedEntryCount = 0;

        if (lcd->bgwEnable())
        {
            fifo.bgwFetchData[0] = vramRead(lcd->bgTileMapArea() +
                                                (fifo.mapX / 8) +
                                                (((fifo.mapY / 8)) * 32),
                                            true);

            if (lcd->bgAndWinTileDataArea() == 0x8800)
            {
                fifo.bgwFetchData[0] += 128;
            }

            pipelineLoadWindowTile();
        }

        if (lcd->objEnable() && lineSprites)
        {
            pipelineLoadSpriteTile();
        }

        fifo.fetchState = FS_DATA0;
        fifo.fetchX += 8;
    }
    break;

    case FS_DATA0:
    {
        fifo.bgwFetchData[1] = vramRead(lcd->bgAndWinTileDataArea() +
                                            (fifo.bgwFetchData[0] * 16) + fifo.tileY,
                                        true);

        pipelineLoadSpriteData(0);

        fifo.fetchState = FS_DATA1;
    }
    break;

    case FS_DATA1:
    {
        fifo.bgwFetchData[2] = vramRead(lcd->bgAndWinTileDataArea() +
                                            (fifo.bgwFetchData[0] * 16) + fifo.tileY + 1,
                                        true);

        pipelineLoadSpriteData(1);

        fifo.fetchState = FS_SLEEP;
    }
    break;

    case FS_SLEEP:
    {
        fifo.fetchState = FS_PUSH;
    }
    break;

    case FS_PUSH:
    {
        if (pipelineFifoAdd())
        {
            fifo.fetchState = FS_TILE;
        }
    }
    break;
    }
}

void PPU::pipelinePushPixel()
{
    if (fifo.size > 8)
    {
        u8 pixel = fifo.pop();

        if (fifo.lineX >= lcd->getRegisters()->scrollX % 8)
        {
            videoBuffer[fifo.pushedX + lcd->getRegisters()->ly * XRES] = pixel;
            fifo.pushedX++;
        }

        fifo.lineX++;
    }
}

void PPU::pipelineProcess()
{
    fifo.mapY = lcd->getRegisters()->ly + lcd->getRegisters()->scrollY;
    fifo.mapX = fifo.fetchX + lcd->getRegisters()->scrollX;
    fifo.tileY = ((lcd->getRegisters()->ly + lcd->getRegisters()->scrollY) % 8) * 2;

    if (!(lineTicks & 1))
    {
        pipelineFetch();
    }

    pipelinePushPixel();
}

bool PPU::pipelineFifoAdd()
{
    if (fifo.size > 8)
    {
        return false;
    }

    int x = fifo.fetchX - (8 - (lcd->getRegisters()->scrollX % 8));

    for (int i = 0; i < 8; i++)
    {
        int bit = 7 - i;
        u8 hi = !!(fifo.bgwFetchData[1] & (1 << bit));
        u8 lo = !!(fifo.bgwFetchData[2] & (1 << bit)) << 1;
        u8 color = lcd->getRegisters()->bgColors[hi | lo];

        if (!lcd->bgwEnable())
        {
            color = lcd->getRegisters()->bgColors[0];
        }

        if (lcd->objEnable())
        {
            color = fetchSpritePixels(bit, color, hi | lo);
        }

        // if ((x + i) >= 0) {
        if (x >= 0)
        {
            fifo.push(color);
            fifo.fifoX++;
        }
    }

    return true;
}

u8 *PPU::getVideoBuffer()
{
    return videoBuffer;
}

u8 *PPU::getVRAM()
{
    return vram;
}
