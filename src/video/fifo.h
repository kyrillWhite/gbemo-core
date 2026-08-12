#pragma once
#include <array>
#include "cpu/fetch_state.h"

constexpr size_t FIFO_CAPACITY = 32;
constexpr size_t FIFO_MASK = FIFO_CAPACITY - 1;

class FIFO
{
private:
    std::array<u8, FIFO_CAPACITY> buffer;
    size_t headIndex;
    size_t tailIndex;

public:
    FetchState fetchState;

    u32 size;

    u8 lineX;
    u8 pushedX;
    u8 fetchX;
    u8 bgwFetchData[3];
    u8 fetchEntryData[20];
    u8 mapY;
    u8 mapX;
    u8 tileY;
    u8 fifoX;

    FIFO();

    void push(u8 value);
    u8 pop();
    void reset();
};
