#pragma once
#include<atomic>

class spinlock
{
private:
    std::atomic_flag _flag;
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
};