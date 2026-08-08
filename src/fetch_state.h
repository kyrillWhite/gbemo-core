#pragma once
#include "common.h"

enum FetchState : u8
{
    FS_TILE,
    FS_DATA0,
    FS_DATA1,
    FS_SLEEP,
    FS_PUSH,
};