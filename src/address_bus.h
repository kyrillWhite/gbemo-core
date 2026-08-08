#pragma once

#include "common.h"

class AddressBus
{
private:
    u16 address;

public:
    AddressBus();

    u16 getAddress();
    void setAddress(u16 _address);
};
