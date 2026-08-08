#pragma once

#include "common.h"
#include "memory.h"

class DataBus
{
private:
    Memory *memory;

public:
    DataBus();

    u8 readMemory();
    void writeMemory(u8 _data);

    void setMemory(Memory *_memory);
};
