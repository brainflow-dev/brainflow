#pragma once

#include <atomic>

class SpinLock
{

    std::atomic_flag lck = ATOMIC_FLAG_INIT;

public:
    inline void lock ()
    {
        while (lck.test_and_set (std::memory_order_acquire))
        {
        }
    }

    inline void unlock ()
    {
        lck.clear (std::memory_order_release);
    }
};
