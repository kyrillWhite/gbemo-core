#include "system/system_clock.h"

SystemClock::SystemClock(int _clockPerSec, bool _unlimit) : systemClockTime(1000000000 / _clockPerSec),
                                                            unlimit(_unlimit),
                                                            delay(0),
                                                            prevTickTime(Time::now())
{
}

long long SystemClock::getDelay()
{
    return delay.count();
}

bool SystemClock::isAllowTick()
{
    if (unlimit)
    {
        return true;
    }

    auto now = Time::now();
    auto tickTime = now - prevTickTime;

    if (tickTime + delay < systemClockTime)
    {
        return false;
    }
    else
    {
        delay = tickTime + delay - systemClockTime;
        prevTickTime = now;
        return true;
    }
}

void SystemClock::reset()
{
    delay = std::chrono::steady_clock::duration(0);
    prevTickTime = Time::now();
}
