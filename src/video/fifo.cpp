#include "video/fifo.h"

FIFO::FIFO() : fetchState(FS_TILE),
               head(nullptr),
               tail(nullptr),
               size(0),
               lineX(0),
               pushedX(0),
               fetchX(0),
               bgwFetchData{0, 0, 0},
               fetchEntryData{0, 0, 0, 0, 0, 0},
               mapY(0),
               mapX(0),
               tileY(0),
               fifoX(0)
{
}

void FIFO::push(u8 value)
{
    FifoEntry *next = new FifoEntry();
    next->next = nullptr;
    next->pixel = value;

    if (!head)
    {
        head = next;
        tail = next;
    }
    else
    {
        tail->next = next;
        tail = next;
    }

    size++;
}

u8 FIFO::pop()
{
    if (size <= 0)
    {
        return 0;
    }

    u8 value = head->pixel;
    FifoEntry *popped = head;
    head = head->next;

    delete popped;

    size--;
    return value;
}

void FIFO::reset()
{
    while (size)
    {
        pop();
    }

    head = nullptr;
}
