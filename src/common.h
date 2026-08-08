#pragma once

#include <stdlib.h>
#include <stdexcept>

#define DEBUG_START 0
// #define DEBUG
// #define DELAY_PRINT
#define PRINT_SKIP 1000000

#define NOT_AVAILABLE throw std::runtime_error("Not available");

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef signed short i16;
typedef signed int i32;

static u16 reverse(u16 v) { return (v << 8 & 0xFF00) | (v >> 8 & 0x00FF); }
static bool bit(u8 v, u8 bit) { return v >> bit & 1; }
static bool bit(u16 v, u16 bit) { return v >> bit & 1; }
static u8 set_bit(u8 v, bool bitV, u8 bit)
{
    return bitV ? (v | 1 << bit) : (v & ~(1 << bit));
}
static u16 set_bit(u16 v, bool bitV, u16 bit)
{
    return bitV ? (v | 1 << bit) : (v & ~(1 << bit));
}

extern unsigned long long globalTicks;
