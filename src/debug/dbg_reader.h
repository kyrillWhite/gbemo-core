#pragma once
#include "memory/memory.h"

class DbgReader
{
private:
    Memory *memory;

    u32 printed;

public:
    DbgReader(Memory *_memory);

    void update();
    void print();
};
