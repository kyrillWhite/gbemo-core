#pragma once
#include "cpu/fetch_state.h"

struct FifoEntry
{
    FifoEntry *next;
    u8 pixel;
};

class FIFO
{
public:
    FetchState fetchState;

    FifoEntry *head;
    FifoEntry *tail;
    u32 size;

    u8 lineX;
    u8 pushedX;
    u8 fetchX;
    u8 bgwFetchData[3];
    u8 fetchEntryData[6];
    u8 mapY;
    u8 mapX;
    u8 tileY;
    u8 fifoX;

    FIFO();

    void push(u8 value);
    u8 pop();

    void reset();
};
