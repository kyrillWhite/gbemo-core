#pragma once

#include "address_bus.h"
#include "data_bus.h"

class Bus
{
private:
    AddressBus* addrBus;
    DataBus* dataBus;

public:
    Bus();
    ~Bus();

    void setAddress(u16 address);
    u8 readMemory();
    void writeMemory(u8 data);

    void setMemory(Memory* memory);
    AddressBus* getAddressBus();
};

