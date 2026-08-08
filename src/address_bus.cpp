#include "address_bus.h"

AddressBus::AddressBus() : address(0x0000)
{}

/* Used by memory to identify necessary location of stored data */
u16 AddressBus::getAddress()
{
    return address;
}

void AddressBus::setAddress(u16 _address)
{
    address = _address;
}
