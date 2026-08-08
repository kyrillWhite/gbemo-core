#include "bus.h"

Bus::Bus() : addrBus(new AddressBus()),
             dataBus(new DataBus())
{
}

u8 Bus::readMemory()
{
    return dataBus->readMemory();
}

void Bus::writeMemory(u8 data)
{
    dataBus->writeMemory(data);
}

void Bus::setMemory(Memory *memory)
{
    dataBus->setMemory(memory);
}

AddressBus *Bus::getAddressBus()
{
    return addrBus;
}

Bus::~Bus()
{
    delete addrBus;
    delete dataBus;
}

void Bus::setAddress(u16 address)
{
    addrBus->setAddress(address);
}
