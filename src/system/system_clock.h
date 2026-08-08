#pragma once

#include <chrono>

#define DEFAULT_SYSTEM_CLOCK_PER_SEC 0b10000000000000000000000

typedef std::chrono::steady_clock Time;

class SystemClock
{
private:
    bool unlimit;

    std::chrono::steady_clock::duration delay;
    std::chrono::steady_clock::duration systemClockTime;
    std::chrono::steady_clock::time_point prevTickTime;

public:
    SystemClock(int _clockPerSec = DEFAULT_SYSTEM_CLOCK_PER_SEC, bool _unlimit = false);

    long long getDelay();
    bool isAllowTick();
    void reset();
};