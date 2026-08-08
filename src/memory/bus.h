#pragma once

#include "memory/address_bus.h"
#include "memory/data_bus.h"

class Bus
{
private:
    AddressBus addrBus;
    DataBus dataBus;

public:
    Bus();

    void setAddress(u16 address);
    u8 readMemory();
    void writeMemory(u8 data);

    void setMemory(Memory *memory);
    AddressBus *getAddressBus();
};
