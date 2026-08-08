#include "video/ppu_sm.h"
#include "video/ppu.h"

void PpuStateMachine::incrementLy()
{
    if (ppu->windowVisible() && lcd->getRegisters()->ly >= lcd->getRegisters()->windowY &&
        lcd->getRegisters()->ly < lcd->getRegisters()->windowY + YRES)
    {
        ppu->windowLine++;
    }

    auto registers = lcd->getRegisters();
    registers->ly++;

    if (registers->ly == registers->lyCompare)
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
    if (ppu->getLineTicks() >= 80)
    {
        lcd->modeSet(XFER);

        ppu->getFIFO()->fetchState = FS_TILE;
        ppu->getFIFO()->lineX = 0;
        ppu->getFIFO()->fetchX = 0;
        ppu->getFIFO()->pushedX = 0;
        ppu->getFIFO()->fifoX = 0;
    }

    if (ppu->getLineTicks() == 1)
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
    if (ppu->getLineTicks() >= TICKS_PER_LINE)
    {
        incrementLy();

        if (lcd->getRegisters()->ly >= LINES_PER_FRAME)
        {
            lcd->modeSet(OAM);
            lcd->getRegisters()->ly = 0;
            ppu->windowLine = 0;
        }
        ppu->setLineTicks(0);
    }
}

void PpuStateMachine::hblankMode()
{
    if (ppu->getLineTicks() >= TICKS_PER_LINE)
    {
        incrementLy();

        if (lcd->getRegisters()->ly >= YRES)
        {
            lcd->modeSet(VBLANK);

            interrupts->setIFflag(VBlank, 1);

            if (lcd->statInt(SS_VBLANK))
            {
                interrupts->setIFflag(LCD_, 1);
            }

            ppu->setCurrentFrame(ppu->getCurrentFrame() + 1);
        }
        else
        {
            lcd->modeSet(OAM);
        }

        ppu->setLineTicks(0);
    }
}
