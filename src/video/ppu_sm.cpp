#include "video/ppu_sm.h"
#include "video/ppu.h"

void PpuStateMachine::incrementLy()
{
    // The window's own line counter only moves on lines the window was drawn
    // on, which is what makes it survive a mid-frame WY change.
    if (ppu->windowVisible() && lcd->registers.ly >= lcd->registers.windowY &&
        lcd->registers.ly < YRES)
    {
        ppu->windowLine++;
    }

    lcd->registers.ly++;
}

PpuStateMachine::PpuStateMachine(
    LCD *_lcd,
    PPU *_ppu,
    Interrupts *_interrupts) : lcd(_lcd),
                               ppu(_ppu),
                               interrupts(_interrupts),
                               statLine(false)
{
}

void PpuStateMachine::updateStatLine()
{
    if (!lcd->enable())
    {
        statLine = false;
        return;
    }

    bool coincidence = lcd->registers.ly == lcd->registers.lyCompare;
    lcd->lycSet(coincidence);

    bool line = (coincidence && lcd->statInt(SS_LYC));

    if (!line)
    {
        switch (lcd->mode())
        {
        case HBLANK:
            line = lcd->statInt(SS_HBLANK);
            break;
        case VBLANK:
            line = lcd->statInt(SS_VBLANK);
            break;
        case OAM:
            line = lcd->statInt(SS_OAM);
            break;
        default:
            break;
        }
    }

    if (line && !statLine)
    {
        interrupts->setIFflag(LCD_, true);
    }

    statLine = line;
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

            ppu->currentFrame++;
        }
        else
        {
            lcd->modeSet(OAM);
        }

        ppu->lineTicks = 0;
    }
}
