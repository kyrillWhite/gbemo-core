#include "video/ppu_sm.h"
#include "video/ppu.h"

void PpuStateMachine::incrementLy()
{
    if (ppu->windowVisible() && lcd->registers.ly >= lcd->registers.windowY &&
        lcd->registers.ly < lcd->registers.windowY + YRES)
    {
        ppu->windowLine++;
    }

    lcd->registers.ly++;

    if (lcd->registers.ly == lcd->registers.lyCompare)
    {
        lcd->lycSet(1);

        if (lcd->statInt(SS_LYC))
        {
            interrupts->setIFflag(LCD_, true);
        }
    }
    else
    {
        lcd->lycSet(0);
    }
}

PpuStateMachine::PpuStateMachine(
    LCD *_lcd,
    PPU *_ppu,
    Interrupts *_interrupts) : lcd(_lcd),
                               ppu(_ppu),
                               interrupts(_interrupts)
{
}

void PpuStateMachine::oamMode()
{
    if (ppu->lineTicks >= 80)
    {
        lcd->modeSet(XFER);

        ppu->getFIFO()->fetchState = FS_TILE;
        ppu->getFIFO()->lineX = 0;
        ppu->getFIFO()->fetchX = 0;
        ppu->getFIFO()->pushedX = 0;
        ppu->getFIFO()->fifoX = 0;
    }

    if (ppu->lineTicks == 1)
    { // must do it 40 times with 2 ticks for each oam
        ppu->lineSpriteCount = 0;
        ppu->lineSprites = nullptr;

        ppu->loadLineSpites();
    }
}

void PpuStateMachine::xferMode()
{
    ppu->pipelineProcess();

    if (ppu->getFIFO()->pushedX >= XRES)
    {
        ppu->getFIFO()->reset();

        lcd->modeSet(HBLANK);

        if (lcd->statInt(SS_HBLANK))
        {
            interrupts->setIFflag(LCD_, 1);
        }
    }
}

void PpuStateMachine::vblankMode()
{
    if (ppu->lineTicks >= TICKS_PER_LINE)
    {
        incrementLy();

        if (lcd->registers.ly >= LINES_PER_FRAME)
        {
            lcd->modeSet(OAM);
            lcd->registers.ly = 0;
            ppu->windowLine = 0;
        }
        ppu->lineTicks = 0;
    }
}

void PpuStateMachine::hblankMode()
{
    if (ppu->lineTicks >= TICKS_PER_LINE)
    {
        incrementLy();

        if (lcd->registers.ly >= YRES)
        {
            lcd->modeSet(VBLANK);

            interrupts->setIFflag(VBlank, 1);

            if (lcd->statInt(SS_VBLANK))
            {
                interrupts->setIFflag(LCD_, 1);
            }

            ppu->currentFrame++;
        }
        else
        {
            lcd->modeSet(OAM);
        }

        ppu->lineTicks = 0;
    }
}
