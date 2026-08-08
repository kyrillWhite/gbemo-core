#pragma once
#include "memory.h"
class DbgReader
{
private:
    Memory* memory;

    char message[1024];
    int messageSize;

public:
    DbgReader(Memory* _memory);

    void update();
    void print();
};

