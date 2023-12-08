#pragma once
#include <atomic>
#include "util.h"
#include <bitset>

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

template <size_t MAX_THREADS>
class arrayLock
{
public:

    arrayLock()
    {

    }

    void lock()
    {
        _slot = _current.fetch_add(1 , std::memory_order_relaxed)%MAX_THREADS;
        while(!_array[_slot].val.load(std::memory_order_acquire)){}
    }

    void unlock()
    {
        _array[_slot].val.store(false, std::memory_order_relaxed);
        _array[(_slot + 1)%MAX_THREADS].val.store(true , std::memory_order_release);
    }


private:

    struct flag
    {
        alignas(std::hardware_destructive_interference_size) std::atomic_bool val;
    };
    flag                       _array[MAX_THREADS];
    static thread_local size_t _slot;
    std::atomic<size_t>        _current; 
};