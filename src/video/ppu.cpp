#include "video/ppu.h"
#include <cstring>

PPU::PPU(LCD *_lcd, bool skipBoot) : lcd(_lcd),
                                     currentFrame(0),
                                     lcdWasEnabled(true),
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
    // With LCDC bit 7 clear the controller is stopped, not merely idle: the
    // screen goes blank, LY reads 0 and no mode - so no VBlank and no STAT
    // interrupt - ever comes out of it. Games switch it off to rewrite VRAM
    // in bulk and would otherwise be fed a frame that never happened.
    if (!lcd->enable())
    {
        if (lcdWasEnabled)
        {
            lcdWasEnabled = false;
            lineTicks = 0;
            windowLine = 0;
            lcd->registers.ly = 0;
            lcd->modeSet(HBLANK);
            lcd->lycSet(false);
            fifo.reset();
            memset(videoBuffer, 0, YRES * XRES);
            ppuSm->updateStatLine();
        }
        return;
    }

    if (!lcdWasEnabled)
    {
        lcdWasEnabled = true;
        lineTicks = 0;
        windowLine = 0;
        lcd->registers.ly = 0;
        lcd->modeSet(OAM);
    }

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

    // STAT is level-triggered off whatever the mode, LY and the enable bits
    // say right now, so it has to be re-evaluated every tick - the CPU can
    // change the enable bits mid-line just as easily as the PPU changes mode.
    ppuSm->updateStatLine();
}

bool PPU::windowVisible()
{
    return lcd->windowEnable() && lcd->registers.windowX <= 166 &&
           lcd->registers.windowY < YRES;
}

void PPU::loadLineSpites()
{
    lineSpriteCount = 0;
    lineSprites = nullptr;

    int curY = lcd->registers.ly;
    u8 sprite_height = lcd->objSize();

    for (int i = 0; i < 40; i++)
    {
        if (lineSpriteCount >= MAX_LINE_SPRITES)
            break;

        OAMEntry e = oam[i];

        // Selection looks at Y and nothing else. A sprite parked at X=0 draws
        // nothing, but it still takes one of the ten slots - which is exactly
        // how games hide a sprite and why dropping it here would let an
        // eleventh one appear that hardware never shows.
        if (e.yPos <= curY + 16 && e.yPos + sprite_height > curY + 16)
        {
            auto *entry = &lineEntryArray[lineSpriteCount++];
            entry->oam = e;
            entry->next = nullptr;

            // Sorted by X ascending, and strictly so: equal X is broken by OAM
            // index, and the earlier index is already in the list.
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
                    if (e.xPos < cur->oam.xPos)
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
    return vram[(address - 0x8000) & 0x1FFF];
}

void PPU::vramWrite(u16 address, u8 value, bool inner)
{
    vram[(address - 0x8000) & 0x1FFF] = value;
}

FIFO *PPU::getFIFO()
{
    return &fifo;
}

u8 PPU::fetchSpritePixels(u8 color, u8 bgColor)
{
    for (int i = 0; i < fetchedEntryCount; i++)
    {
        int spX = (fetchedEntries[i].xPos - 8) +
                  ((lcd->registers.scrollX % 8));

        if (spX + 8 < fifo.fifoX)
        {
            continue;
        }

        int offset = fifo.fifoX - spX;

        if (offset < 0 || offset > 7)
        {
            continue;
        }

        int spriteBit = fetchedEntries[i].xFlip ? offset : (7 - offset);

        u8 lo = !!(fifo.fetchEntryData[i * 2] & (1 << spriteBit));
        u8 hi = !!(fifo.fetchEntryData[(i * 2) + 1] & (1 << spriteBit)) << 1;
        u8 index = hi | lo;

        // Colour 0 is transparent, so this sprite does not claim the pixel and
        // the next one along still gets its turn.
        if (!index)
        {
            continue;
        }

        // Everything after this point belongs to this sprite alone. The list
        // is in priority order, so the first one with an opaque pixel wins the
        // slot outright: if its priority bit then loses to the background, the
        // background is what shows - not the sprite queued behind it. Letting
        // that one through is what puts overlapping objects in the wrong
        // order.
        if (!fetchedEntries[i].priority || bgColor == 0)
        {
            color = (fetchedEntries[i].dmgPalette) ? lcd->registers.sp2Colors[index] : lcd->registers.sp1Colors[index];
        }

        break;
    }

    return color;
}

void PPU::pipelineLoadWindowTile()
{
    if (!windowVisible())
    {
        return;
    }

    u8 window_y = lcd->registers.windowY;

    if (fifo.fetchX + 7 >= lcd->registers.windowX && lcd->registers.ly >= window_y)
    {
        fifo.bgwFetchData[0] = vramRead(lcd->winTileMapArea() +
                                            ((fifo.fetchX + 7 - lcd->registers.windowX) / 8) +
                                            ((windowLine / 8) * 32),
                                        true);

        // The window has its own line counter, which only advances on the
        // lines it is actually drawn on. Indexing its rows with the background
        // one instead shears the window apart whenever SCY is not a multiple
        // of eight.
        fifo.tileY = (windowLine % 8) * 2;

        if (lcd->bgAndWinTileDataArea() == 0x8800)
        {
            fifo.bgwFetchData[0] += 128;
        }
    }
}

void PPU::pipelineLoadSpriteData(u8 offset)
{
    int curY = lcd->registers.ly;
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

    while (le && fetchedEntryCount < MAX_LINE_SPRITES)
    {
        int spX = (le->oam.xPos - 8) + (lcd->registers.scrollX % 8);

        if ((spX >= fifo.fetchX && spX < fifo.fetchX + 8) ||
            ((spX + 8) >= fifo.fetchX && (spX + 8) < fifo.fetchX + 8))
        {
            fetchedEntries[fetchedEntryCount++] = le->oam;
        }

        le = le->next;
    }
}

void PPU::pipelineFetch()
{
    switch (fifo.fetchState)
    {
    case FS_TILE:
    {
        fetchedEntryCount = 0;

        // Fixed here, where the tile index is read, rather than every tick:
        // the window fetch below replaces it with its own row and must not be
        // overwritten before the two bitplanes are read.
        fifo.tileY = ((lcd->registers.ly + lcd->registers.scrollY) % 8) * 2;

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

        if (fifo.lineX >= lcd->registers.scrollX % 8)
        {
            videoBuffer[fifo.pushedX + lcd->registers.ly * XRES] = pixel;
            fifo.pushedX++;
        }

        fifo.lineX++;
    }
}

void PPU::pipelineProcess()
{
    fifo.mapY = lcd->registers.ly + lcd->registers.scrollY;
    fifo.mapX = fifo.fetchX + lcd->registers.scrollX;

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

    int x = fifo.fetchX - (8 - (lcd->registers.scrollX % 8));

    for (int i = 0; i < 8; i++)
    {
        int bit = 7 - i;
        u8 hi = !!(fifo.bgwFetchData[1] & (1 << bit));
        u8 lo = !!(fifo.bgwFetchData[2] & (1 << bit)) << 1;
        u8 bgIndex = hi | lo;
        u8 color = lcd->registers.bgColors[bgIndex];

        // With LCDC bit 0 clear the background is not merely blank, it is gone:
        // colour 0 everywhere, and sprites draw over it whatever their priority
        // bit says. Handing the stale fetch data to the sprite mixer instead
        // makes priority sprites vanish behind a background that is not there.
        if (!lcd->bgwEnable())
        {
            bgIndex = 0;
            color = lcd->registers.bgColors[0];
        }

        if (lcd->objEnable())
        {
            color = fetchSpritePixels(color, bgIndex);
        }

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
