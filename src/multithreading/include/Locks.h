#pragma once
#include<atomic>
#include"util.h"

class spinlock
{
public:

    spinlock(){_flag.clear(std::memory_order_relaxed);}

    void lock()
    {
        while(_flag.test_and_set(std::memory_order_acquire)){}
    }

    void unlock()
    {
        _flag.clear(std::memory_order_release);
    }
private:
    std::atomic_flag _flag;
};

class betterSpinLock
{
public:
    betterSpinLock(){_flag.store(true, std::memory_order_relaxed);}

    void lock()
    {
        while(true)
        {
            if(_flag.load(std::memory_order_relaxed))
            {
                auto val = true;
                if(_flag.compare_exchange_strong(val, false , std::memory_order_acquire, std::memory_order_relaxed))
                    break;
            }
        }
    }

    void unlock()
    {
        _flag.store(true, std::memory_order_release);
    }
private:
    std::atomic_bool _flag;
};

class backoffLock
{
public:
    void lock()
    {
        backOff
    }

private:
    std::atomic_bool _flag;
};