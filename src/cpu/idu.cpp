#include "cpu/idu.h"

IDU::IDU()
{
}

u16 IDU::increment(u16 value)
{
    return ++value;
}

u16 IDU::decrement(u16 value)
{
    return --value;
}
