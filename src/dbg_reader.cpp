#include "dbg_reader.h"

DbgReader::DbgReader(Memory* _memory): memory(_memory), messageSize(0)
{
}

void DbgReader::update()
{
    if (memory->read(0xFF02) == 0x81) {
        char c = memory->read(0xFF01);

        message[messageSize++] = c;

        memory->write(0xFF02, 0);
    }
}

void DbgReader::print()
{
    if (message[0] && globalTicks % (PRINT_SKIP + 1) == 0) {
        printf("DBG: %s\n", message);
    }
}
