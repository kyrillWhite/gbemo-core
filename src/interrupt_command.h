#include "common.h"
#include "interrupt_type.h"

struct InterruptCommand {
    InterruptType type;
    u16 address;
    bool isHandling;
};