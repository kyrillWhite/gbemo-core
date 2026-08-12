#include <cstdio>
#include "debug/dbg_reader.h"

DbgReader::DbgReader(Memory *_memory) : memory(_memory), printed(0)
{
}

void DbgReader::update()
{
    u32 size = memory->getSerialLogSize();
    if (printed >= size)
    {
        return;
    }

    const char *log = memory->getSerialLog();
    while (printed < size)
    {
        char c = log[printed++];
        // Games that poke the link port write arbitrary bytes; only the text
        // a test ROM sends is worth putting on a terminal.
        if (c == '\n' || (c >= 0x20 && c < 0x7F))
        {
            putchar(c);
        }
    }
    fflush(stdout);
}

void DbgReader::print()
{
}
