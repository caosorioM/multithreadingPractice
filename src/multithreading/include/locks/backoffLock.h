#pragma once
#include <atomic>
#include "../util/backoff.h"

class backoffLock
{
public:
    void lock()
    {
        backOff<std::chrono::microseconds , 1, 100 > theBackOff;

        while(true)
        {
            if(_flag.load(std::memory_order_relaxed))
            {
                auto val = false;
                if(_flag.compare_exchange_strong(val, std::memory_order_acquire, std::memory_order_relaxed))
                {
                    break;
                }
            }
            theBackOff.sleep();
        }
    }

    void unlock()
    {
        _flag.store(true, std::memory_order_release);
    }

private:
    std::atomic_bool _flag;
};