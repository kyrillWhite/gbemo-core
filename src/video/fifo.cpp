#include "video/fifo.h"

FIFO::FIFO() : fetchState(FS_TILE),
               headIndex(0),
               tailIndex(0),
               size(0),
               lineX(0),
               pushedX(0),
               fetchX(0),
               bgwFetchData{0, 0, 0},
               fetchEntryData{},
               mapY(0),
               mapX(0),
               tileY(0),
               fifoX(0)
{
    buffer.fill(0);
}

void FIFO::push(u8 value)
{
    if (size >= FIFO_CAPACITY)
    {
        return;
    }

    buffer[tailIndex] = value;
    tailIndex = (tailIndex + 1) & FIFO_MASK;
    size++;
}

u8 FIFO::pop()
{
    if (size == 0)
    {
        return 0;
    }

    u8 value = buffer[headIndex];
    headIndex = (headIndex + 1) & FIFO_MASK;

    size--;
    return value;
}

void FIFO::reset()
{
    headIndex = 0;
    tailIndex = 0;
    size = 0;
}
